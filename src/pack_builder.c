#define WIDTH_DATA_OFFSET  20  // Offset to width data with BFF file
#define MAP_DATA_OFFSET   276  // Offset to texture image data with BFF file

#define BFG_RS_NONE  0x0  // Blend flags
#define BFG_RS_ALPHA 0x1
#define BFG_RS_RGB   0x2
#define BFG_RS_RGBA  0x4

#define MESH_VERTICES_MAX 200000

// TODO: Tune these as necessary
#define MAX_PACK_BLOBS 4096
#define MAX_PACK_BUF_SIZE MB(512)
#define DEFAULT_PACK_BLOBS 128
#define DEFAULT_PACK_BUF_SIZE MB(4)

internal const u8 PACK_SIGNATURE[4] = { 'R', 'I', 'C', 'O' };
//internal const u32 PACK_SIGNATURE =
//    ((u32)'R' << 24) | ((u32)'I' << 16) | ((u32)'C' << 8) | ((u32)'O');

struct pack *pack_default = 0;
struct pack *pack_active = 0;
struct pack *pack_short = 0;
struct pack *pack_frame = 0;

internal inline u32 blob_start(struct pack *pack, enum rico_hnd_type type)
{
    RICO_ASSERT(pack->blobs_used < pack->blob_count);
    pack->index[pack->blobs_used].type = type;
    pack->index[pack->blobs_used].offset = pack->buffer_used;
    return pack->blobs_used;
}
internal inline u32 blob_offset(struct pack *pack)
{
    return pack_offset(pack, pack->index[pack->blobs_used].offset);
}
internal inline void blob_end(struct pack *pack)
{
    u32 size = blob_offset(pack);
    pack->index[pack->blobs_used].size = size;
    pack->blobs_used++;
}
internal inline void blob_error(struct pack *pack, u32 *pack_idx)
{
    pack_pop(pack, *pack_idx);
    *pack_idx = UINT_MAX;
}

internal u32 load_object(struct pack *pack, const char *name,
                         enum rico_obj_type type, u32 mesh_id, u32 material_id,
                         const struct bbox *bbox)
{
    u32 blob_idx = blob_start(pack, RICO_HND_OBJECT);
    struct rico_object *obj = push_bytes(pack, sizeof(*obj));
    obj->id = blob_idx;
    obj->type = type;
    obj->trans = VEC3_ZERO;
    obj->rot = VEC3_ZERO;
    if (type == OBJ_STRING_SCREEN)
        obj->scale = VEC3_SCALE_ASPECT;
    else
        obj->scale = VEC3_ONE;
    obj->transform = MAT4_IDENT;
    obj->transform_inverse = MAT4_IDENT;
    obj->mesh_id = mesh_id;
    obj->material_id = material_id;
    if (bbox)
        obj->bbox = *bbox;
    object_update_transform(obj);

    obj->name_offset = blob_offset(pack);
    push_string(pack, name);

    blob_end(pack);
    return blob_idx;
}

internal u32 load_texture(struct pack *pack, const char *name,
                          const char *filename, GLenum target)
{
    enum rico_error err = SUCCESS;

    u32 blob_idx = blob_start(pack, RICO_HND_TEXTURE);
    struct rico_texture *tex = push_bytes(pack, sizeof(*tex));
    tex->id = blob_idx;

    // Load raw texture data
    u8 *pixels = stbi_load(filename, &tex->width, &tex->height, NULL, 4);
    if (!pixels)
    {
        err = RICO_ERROR(ERR_TEXTURE_LOAD, "Failed to load texture file: %s",
                         filename);
        goto cleanup;
    }
    tex->bpp = 32;
    tex->gl_target = target;

    tex->name_offset = blob_offset(pack);
    push_string(pack, name);

    tex->pixels_offset = blob_offset(pack);
    push_data(pack, pixels, tex->width * tex->height * (tex->bpp / 8));

    blob_end(pack);

cleanup:
    stbi_image_free(pixels);
    if (err) blob_error(pack, &blob_idx);
    return blob_idx;
}
internal u32 load_font(struct pack *pack, const char *name,
                       const char *filename, u32 *font_tex_diff)
{
    enum rico_error err;

    // Read font file
    char *buffer = NULL;
    int length;
    err = file_contents(filename, &length, &buffer);
    if (err) goto cleanup;

    // Check file signature
    if ((u8)buffer[0] != 0xBF || (u8)buffer[1] != 0xF2)
    {
        err = RICO_ERROR(ERR_FILE_SIGNATURE, "Unexpected file signature");
        goto cleanup;
    }

    // Allocate font/texture
    u32 font_idx = blob_start(pack, RICO_HND_FONT);
    struct rico_font *font = push_bytes(pack, sizeof(*font));
    font->id = font_idx;

    u32 tex_width = 0;
    u32 tex_height = 0;
    u32 tex_bpp = 0;

    // Read bff header
    memcpy(&tex_width, &buffer[2], sizeof(u32));
    memcpy(&tex_height, &buffer[6], sizeof(u32));
    memcpy(&font->cell_x, &buffer[10], sizeof(u32));
    memcpy(&font->cell_y, &buffer[14], sizeof(u32));

    tex_bpp = buffer[18];
    font->base_char = buffer[19];

    // Check filesize
    u32 pixels_size = tex_width * tex_height * (tex_bpp / 8);
    RICO_ASSERT(length == MAP_DATA_OFFSET + pixels_size);

    // Calculate font params
    font->row_pitch = tex_width / font->cell_x;
    font->col_factor = (float)font->cell_x / tex_width;
    font->row_factor = (float)font->cell_y / tex_height;
    font->y_offset = font->cell_y;
    font->y_invert = false;

    // Determine blending options based on BPP
    switch (tex_bpp)
    {
    case 8: // Greyscale
        font->render_style = BFG_RS_ALPHA;
        break;
    case 24: // RGB
        font->render_style = BFG_RS_RGB;
        break;
    case 32: // RGBA
        font->render_style = BFG_RS_RGBA;
        break;
    default: // Unsupported BPP
        goto cleanup;
    }

    // Store character widths
    memcpy(font->char_widths, &buffer[WIDTH_DATA_OFFSET], 256);

    font->name_offset = blob_offset(pack);
    push_string(pack, name);

    blob_end(pack);

    //------------------------------------------------------------

    u32 tex_idx = blob_start(pack, RICO_HND_TEXTURE);
    struct rico_texture *tex = push_bytes(pack, sizeof(*tex));
    tex->id = tex_idx;
    tex->width = tex_width;
    tex->height = tex_height;
    tex->bpp = tex_bpp;
    tex->gl_target = GL_TEXTURE_2D;  // Fonts are always 2D textures

    tex->name_offset = blob_offset(pack);
    push_string(pack, name);

    tex->pixels_offset = blob_offset(pack);
    push_data(pack, &buffer[MAP_DATA_OFFSET], pixels_size);

    blob_end(pack);

    font->texture_id = tex->id;
    *font_tex_diff = font->texture_id;

cleanup:
    free(buffer);
    if (err) blob_error(pack, &font_idx);
    return font_idx;
}
internal u32 load_material(struct pack *pack, const char *name, u32 tex_diff,
                           u32 tex_spec, float shiny)
{
    u32 blob_idx = blob_start(pack, RICO_HND_MATERIAL);
    struct rico_material *mat = push_bytes(pack, sizeof(*mat));

    mat->id = blob_idx;
    mat->shiny = shiny;
    mat->tex_diffuse_id = tex_diff;
    mat->tex_specular_id = tex_spec;

    mat->name_offset = blob_offset(pack);
    push_string(pack, name);

    blob_end(pack);
    return blob_idx;
}
internal u32 load_mesh(struct pack *pack, const char *name, u32 vertex_count,
                       const struct rico_vertex *vertex_data, u32 element_count,
                       const GLuint *element_data)
{
    u32 blob_idx = blob_start(pack, RICO_HND_MESH);
    struct rico_mesh *mesh = push_bytes(pack, sizeof(*mesh));

    mesh->id = blob_idx;
    mesh->vertex_count = vertex_count;
    mesh->element_count = element_count;
    bbox_init_mesh(&mesh->bbox, vertex_data, vertex_count,
                   COLOR_WHITE_HIGHLIGHT);

    mesh->name_offset = blob_offset(pack);
    push_string(pack, name);
    mesh->vertices_offset = blob_offset(pack);
    push_data(pack, vertex_data, vertex_count * sizeof(*vertex_data));
    mesh->elements_offset = blob_offset(pack);
    push_data(pack, element_data, element_count * sizeof(*element_data));
    
    blob_end(pack);
    return blob_idx;
}

internal u32 default_mesh(struct pack *pack, const char *name)
{
    //--------------------------------------------------------------------------
    // Create default mesh (rainbow cube)
    //--------------------------------------------------------------------------
    const struct rico_vertex verts[] = {
        {
            { -1.0f, -1.0f, 1.0f }, // Position
            COLOR_BLACK,            // Color
            { 0.0f, 0.0f, 1.0f },   // Normal
            { 0.0f, 0.0f }          // UV-coords
        },
        {
            { 1.0f, -1.0f, 1.0f },
            COLOR_RED,
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        {
            { 1.0f, 1.0f, 1.0f },
            COLOR_YELLOW,
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f }
        },
        {
            { -1.0f, 1.0f, 1.0f },
            COLOR_GREEN,
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f }
        },
        {
            { -1.0f, -1.0f, -1.0f },
            COLOR_BLUE,
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f }
        },
        {
            { 1.0f, -1.0f, -1.0f },
            COLOR_MAGENTA,
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        {
            { 1.0f, 1.0f, -1.0f },
            COLOR_WHITE,
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f }
        },
        {
            { -1.0f, 1.0f, -1.0f },
            COLOR_CYAN,
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f }
        }
    };
    const GLuint elements[] = {
        0, 1, 2, 2, 3, 0,
        4, 0, 3, 3, 7, 4,
        5, 4, 7, 7, 6, 5,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };

    return load_mesh(pack, name, array_count(verts), verts,
                     array_count(elements), elements);
}

int load_obj_file(struct pack *pack, const char *filename)
{
    enum rico_error err;

    // TODO: Colossal waste of memory, just preprocess the file and count them!
    struct vec3 *positions = calloc(MESH_VERTICES_MAX, sizeof(*positions));
    struct tex2 *texcoords = calloc(MESH_VERTICES_MAX, sizeof(*texcoords));
    struct vec3 *normals = calloc(MESH_VERTICES_MAX, sizeof(*normals));
    struct rico_vertex *vertices = calloc(MESH_VERTICES_MAX, sizeof(*vertices));
    GLuint *elements = calloc(MESH_VERTICES_MAX, sizeof(*elements));

    int idx_pos = 0;
    int idx_tex = 0;
    int idx_normal = 0;
    int idx_vertex = 0;
    int idx_element = 0;
    int idx_mesh = 0;

    long vert_pos = 0;
    long vert_tex = 0;
    long vert_norm = 0;

    int length;
    char *buffer;
    char *tok;

    err = file_contents(filename, &length, &buffer);
    if (err) goto cleanup;

    char *name = NULL;
    char *buffer_ptr = buffer;
    while (*buffer_ptr)
    {
        tok = strsep(&buffer_ptr, '\n');

        // New object
        if (str_starts_with(tok, "o "))
        {
            if (idx_vertex > 0)
            {
                load_mesh(pack, name, idx_vertex, vertices, idx_element,
                          elements);
                idx_mesh++;
            }

            idx_vertex = 0;
            idx_element = 0;
            name = tok + 2;
        }
        else if (str_starts_with(tok, "v "))
        {
            tok += 2;
            positions[idx_pos].x = strtof(tok, &tok);
            positions[idx_pos].y = strtof(tok, &tok);
            positions[idx_pos].z = strtof(tok, &tok);
            if (++idx_pos >= MESH_VERTICES_MAX)
            {
                err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS,
                                 "Too many vertices in mesh %s",
                                 filename);
                goto cleanup;
            }
        }
        else if (str_starts_with(tok, "vt "))
        {
            tok += 3;
            texcoords[idx_tex].u = strtof(tok, &tok);
            texcoords[idx_tex].v = strtof(tok, &tok);
            if (++idx_tex >= MESH_VERTICES_MAX)
            {
                err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS,
                                 "Too many tex coords in mesh %s",
                                 filename);
                goto cleanup;
            }
        }
        else if (str_starts_with(tok, "vn "))
        {
            tok += 3;
            normals[idx_normal].x = strtof(tok, &tok);
            normals[idx_normal].y = strtof(tok, &tok);
            normals[idx_normal].z = strtof(tok, &tok);
            if (++idx_normal >= MESH_VERTICES_MAX)
            {
                err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS,
                                 "Too many normals in mesh %s",
                                 filename);
                goto cleanup;
            }
        }
        else if (str_starts_with(tok, "f "))
        {
            char *tok_ptr = tok + 2;
            char *vert;
            while (*tok_ptr)
            {
                vert = strsep(&tok_ptr, ' ');
                vert_pos = fast_atol(strsep(&vert, '/'));
                vert_tex = fast_atol(strsep(&vert, '/'));
                vert_norm = fast_atol(strsep(&vert, '/'));

                vertices[idx_vertex].col = COLOR_WHITE;
                if (vert_pos > 0)
                    vertices[idx_vertex].pos = positions[vert_pos - 1];
                if (vert_tex > 0)
                    vertices[idx_vertex].uv = texcoords[vert_tex - 1];
                if (vert_norm > 0)
                    vertices[idx_vertex].normal = normals[vert_norm - 1];
                if (++idx_vertex >= MESH_VERTICES_MAX)
                {
                    err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS,
                                     "Too many vertices in mesh %s",
                                     filename);
                    goto cleanup;
                }

                elements[idx_element] = idx_vertex - 1;
                if (++idx_element >= MESH_VERTICES_MAX)
                {
                    err = RICO_ERROR(ERR_OBJ_TOO_MANY_VERTS,
                                     "Too many indicies in mesh %s",
                                     filename);
                    goto cleanup;
                }
            }
        }
    }

    if (idx_vertex > 0)
    {
        load_mesh(pack, name, idx_vertex, vertices, idx_element, elements);
        idx_mesh++;
    }

cleanup:
    if (buffer)     free(buffer);
    if (positions)  free(positions);
    if (texcoords)  free(texcoords);
    if (normals)    free(normals);
    if (vertices)   free(vertices);
    if (elements)   free(elements);
    return err;
}

internal u32 null_blob(struct pack *pack)
{
    u32 blob_idx = blob_start(pack, RICO_HND_NULL);
    push_string(pack, "[This page intentionally left blank]");
    blob_end(pack);
    return blob_idx;
}

internal u32 perf_pack_tick_start;
internal u32 perf_pack_tick_end;

struct pack *pack_init(const char *name, u32 blob_count, u32 buffer_size)
{
    if (!blob_count) blob_count = DEFAULT_PACK_BLOBS;
    if (!buffer_size) buffer_size = DEFAULT_PACK_BUF_SIZE;
    RICO_ASSERT(blob_count < MAX_PACK_BLOBS);
    RICO_ASSERT(buffer_size < MAX_PACK_BUF_SIZE);

    perf_pack_tick_start = SDL_GetTicks();

    struct pack *pack = calloc(1,
        sizeof(struct pack) +
        blob_count * sizeof(struct blob_index) +
        buffer_size);
    
    memcpy(pack->magic, PACK_SIGNATURE, sizeof(pack->magic));
    pack->version = RICO_FILE_VERSION_CURRENT;
    strncpy(pack->name, name, sizeof(pack->name) - 1);
    pack->blob_count = blob_count;
    pack->blobs_used = 0;
    pack->buffer_size = buffer_size;
    pack->buffer_used = 0;
    pack->index_offset = sizeof(struct pack);
    pack->data_offset = pack->index_offset +
                        pack->blob_count * sizeof(*pack->index);
    pack->index = (struct blob_index *)((u8 *)pack + pack->index_offset);
    pack->buffer = (u8 *)pack + pack->data_offset;

    // First blob in pack must be the null blob
    null_blob(pack);

    return pack;
}

int pack_save(struct pack *pack, const char *filename, bool compact)
{
    enum rico_error err = SUCCESS;

    // Write pack file to disk
    FILE *pack_file = fopen(filename, "wb");
    if (pack_file)
    {
        u32 blob_count = pack->blob_count;
        u32 buffer_size = pack->buffer_size;
        u32 data_offset = pack->data_offset;

        if (compact)
        {
            // Remove unused blob entires / buffer space for the save
            pack->blob_count = pack->blobs_used;
            pack->buffer_size = pack->buffer_used;
            pack->data_offset = sizeof(struct pack) +
                                pack->blob_count * sizeof(struct blob_index);
        }

        u32 pack_size = sizeof(struct pack) +
                        pack->blob_count * sizeof(struct blob_index) +
                        pack->buffer_size;

        u32 bytes_written = 0;
        bytes_written += fwrite(pack, 1, sizeof(*pack), pack_file);
        bytes_written += fwrite(pack->index, 1,
                                pack->blob_count * sizeof(*pack->index),
                                pack_file);
        bytes_written += fwrite(pack->buffer, 1, pack->buffer_size, pack_file);
        fclose(pack_file);
        RICO_ASSERT(bytes_written == pack_size);

        if (compact)
        {
            // Restore actual blob count / buffer size in memory
            pack->blob_count = blob_count;
            pack->buffer_size = buffer_size;
            pack->data_offset = data_offset;
        }
    }
    else
    {
        err = RICO_ERROR(ERR_FILE_WRITE,
                         "Error: Cannot open pack file [%s] for read.\n",
                         filename);
    }

    perf_pack_tick_end = SDL_GetTicks();
    printf("[PERF][pack] '%s' built in: %u ticks\n", filename,
           perf_pack_tick_end - perf_pack_tick_start);
    perf_pack_tick_start = perf_pack_tick_end = 0;
    return err;
}

int pack_load(const char *filename, struct pack **_pack)
{
    UNUSED(filename);
    enum rico_error err = SUCCESS;
    u32 tick_start = SDL_GetTicks();

    // Read pack file from disk
    FILE *pack_file = fopen(filename, "rb");
    if (pack_file)
    {
        struct pack tmp_pack = { 0 };
        fread(&tmp_pack, 1, sizeof(tmp_pack), pack_file);
        u32 pack_size = tmp_pack.data_offset + tmp_pack.buffer_size;
        fseek(pack_file, 0, SEEK_SET);

        struct pack *pack = calloc(1, pack_size);
        u32 bytes_read = fread(pack, 1, pack_size, pack_file);
        fclose(pack_file);
        RICO_ASSERT(bytes_read == pack_size);

        pack->index = (struct blob_index *)((u8 *)pack + pack->index_offset);
        pack->buffer = (u8 *)pack + pack->data_offset;
        *_pack = pack;
    }
    else
    {
        err = RICO_ERROR(ERR_FILE_WRITE,
                         "Error: Cannot open pack file [%s] for write.\n",
                         filename);
        goto cleanup;
    }

    u32 tick_end = SDL_GetTicks();
    printf("[PERF][pack] Pack read in: %d ticks\n", tick_end - tick_start);

cleanup:
    return err;
}

void pack_build_default()
{
    // TODO: This entire pack could be embedded as binary data in the .exe once
    //       the contents are finalized. This would allow the engine to run even
    //       when the data directory is missing.
    const char *filename = "packs/default.pak";

    struct pack *pack = pack_init(filename, 10, MB(1));
    u32 font_tex_diff = 0;
    u32 font = load_font(pack, "[FONT_DEFAULT]", "font/courier_new.bff",
                         &font_tex_diff);
    u32 diff = load_texture(pack, "[TEX_DIFF_DEFAULT]",
                            "texture/basic_diff.tga", GL_TEXTURE_2D);
    u32 spec = load_texture(pack, "[TEX_SPEC_DEFAULT]",
                            "texture/basic_spec.tga", GL_TEXTURE_2D);
    u32 mat  = load_material(pack, "[MATERIAL_DEFAULT]", diff, spec, 0.5f);
    u32 font_mat = load_material(pack, "[FONT_DEFAULT_MATERIAL]", font_tex_diff,
                                 0, 0.0f);
    u32 bbox = default_mesh(pack, "[PRIM_MESH_BBOX]");

    // HACK: This is a bit of a gross way to assert that the obj file only
    //       contained a single mesh: the sphere primitive.
    load_obj_file(pack, "mesh/prim_sphere.ric");
    u32 sphere = pack->blobs_used - 1;

    pack_save(pack, filename, true);
    free(pack);

    RICO_ASSERT(font == FONT_DEFAULT);
    RICO_ASSERT(font_tex_diff == FONT_DEFAULT_TEX_DIFF);
    RICO_ASSERT(diff == TEXTURE_DEFAULT_DIFF);
    RICO_ASSERT(spec == TEXTURE_DEFAULT_SPEC);
    RICO_ASSERT(mat  == MATERIAL_DEFAULT);
    RICO_ASSERT(font_mat == FONT_DEFAULT_MATERIAL);
    RICO_ASSERT(bbox == MESH_DEFAULT_BBOX);
    RICO_ASSERT(sphere == MESH_DEFAULT_SPHERE);
}

void pack_build_alpha()
{
    const char *filename = "packs/alpha.pak";

    struct pack *pack = pack_init(filename, 16, MB(1));
    load_obj_file(pack, "mesh/sphere.ric");
    load_obj_file(pack, "mesh/wall_cornertest.ric");
    //load_obj_file(&pack, "mesh/conference.ric");
    //load_obj_file(&pack, "mesh/spawn.ric");
    //load_obj_file(&pack, "mesh/door.ric");
    //load_obj_file(&pack, "mesh/welcome_floor.ric");
    //load_obj_file(&pack, "mesh/grass.ric");
    pack_save(pack, filename, false);
    free(pack);
}

void pack_build_all()
{
    pack_build_default();
    pack_build_alpha();
}