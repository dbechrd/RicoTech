#define WIDTH_DATA_OFFSET  20  // Offset to width data with BFF file
#define MAP_DATA_OFFSET   276  // Offset to texture image data with BFF file

#define BFG_RS_NONE  0x0  // Blend flags
#define BFG_RS_ALPHA 0x1
#define BFG_RS_RGB   0x2
#define BFG_RS_RGBA  0x4

#define MESH_VERTICES_MAX 200000
#define MAX_PACK_ENTRIES 1024
#define MAX_PACK_SIZE 1024 * 1024 * 512 // 512 MB

internal const u32 PACK_SIGNATURE =
    ((u32)'R' << 24) | ((u32)'I' << 16) | ((u32)'C' << 8) | ((u32)'O');

struct pack *pack_active = 0;
struct pack *pack_frame = 0;

internal inline u32 blob_start(struct pack *pack, enum rico_hnd_type type)
{
    pack->index[pack->blob_count].type = type;
    pack->index[pack->blob_count].offset = pack->buffer_used;
    return pack->blob_count;
}
internal inline u32 blob_offset(struct pack *pack)
{
    return pack_offset(pack, pack->index[pack->blob_count].offset);
}
internal inline void blob_end(struct pack *pack)
{
    u32 size = blob_offset(pack);
    pack->index[pack->blob_count].size = size;
    pack->buffer_used += size;
    pack->blob_count++;
}
internal inline void blob_error(struct pack *pack, u32 *pack_idx)
{
    pack_pop(pack, *pack_idx);
    *pack_idx = UINT_MAX;
}

internal u32 load_texture(struct pack *pack, const char *name,
                          const char *filename)
{
    enum rico_error err;

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

    tex->name_offset = blob_offset(pack);
    push_string(pack, name);

    tex->pixels_offset = blob_offset(pack);
    u32 data_size = tex->width * tex->height * (tex->bpp / 8);
    void *tex_pixels = push_bytes(pack, data_size);
    memcpy(tex_pixels, pixels, data_size);

    blob_end(pack);

cleanup:
    stbi_image_free(pixels);
    if (err) blob_error(pack, &blob_idx);
    return blob_idx;
}
internal u32 load_font(struct pack *pack, const char *name,
                       const char *filename)
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
    u32 blob_idx = blob_start(pack, RICO_HND_FONT);
    struct rico_font *font = push_bytes(pack, sizeof(*font));
    font->id = blob_idx;

    font->name_offset = blob_offset(pack);
    push_string(pack, name);

    font->texture_offset = blob_offset(pack);
    struct rico_texture *tex = push_bytes(pack, sizeof(*tex));

    font->InvertYAxis = false;

    // Read bff header
    memcpy(&tex->width, &buffer[2], sizeof(int));
    memcpy(&tex->height, &buffer[6], sizeof(int));
    memcpy(&font->CellX, &buffer[10], sizeof(int));
    memcpy(&font->CellY, &buffer[14], sizeof(int));

    tex->bpp = buffer[18];
    font->Base = buffer[19];

    // Check filesize
    u32 data_size = tex->width * tex->height * (tex->bpp / 8);
    RICO_ASSERT(length == MAP_DATA_OFFSET + data_size);

    // Calculate font params
    font->RowPitch = tex->width / font->CellX;
    font->ColFactor = (float)font->CellX / tex->width;
    font->RowFactor = (float)font->CellY / tex->height;
    font->YOffset = font->CellY;

    // Determine blending options based on BPP
    switch (tex->bpp)
    {
    case 8: // Greyscale
        font->RenderStyle = BFG_RS_ALPHA;
        break;
    case 24: // RGB
        font->RenderStyle = BFG_RS_RGB;
        break;
    case 32: // RGBA
        font->RenderStyle = BFG_RS_RGBA;
        break;
    default: // Unsupported BPP
        goto cleanup;
    }

    // Store character widths
    memcpy(font->Width, &buffer[WIDTH_DATA_OFFSET], 256);

    // Store pixels
    tex->pixels_offset = blob_offset(pack);
    void *tex_pixels = push_bytes(pack, data_size);
    memcpy(tex_pixels, &buffer[MAP_DATA_OFFSET], data_size);

    blob_end(pack);

cleanup:
    free(buffer);
    if (err) blob_error(pack, &blob_idx);
    return blob_idx;
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
    push_bytes(pack, vertex_count * sizeof(*vertex_data));
    mesh->elements_offset = blob_offset(pack);
    push_bytes(pack, element_count * sizeof(*element_data));
    
    blob_end(pack);
    return blob_idx;
}

internal u32 default_mesh(struct pack *pack)
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

    return load_mesh(pack, "[PRIM_MESH_BBOX]", array_count(verts), verts,
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

    printf("[ obj][load] filename=%s\n", filename);
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

int pack_build(const char *filename)
{
    UNUSED(filename);
    enum rico_error err;
    

    u32 tick_start = SDL_GetTicks();

    struct pack pack = { 0 };
    pack.magic = PACK_SIGNATURE;
    pack.version = RICO_FILE_VERSION_CURRENT;
    pack.index = calloc(1024, sizeof(*pack.index));
    pack.buffer_size = 1024 * 1024 * 512; // 512 MB
    pack.buffer = calloc(1, pack.buffer_size);

    // First blob in pack must be the null blob
    null_blob(&pack);

    // Load assets
    load_font(&pack, "[FONT_DEFAULT]", "font/courier_new.bff");

    u32 diff = load_texture(&pack, "[TEX_DIFF_DEFAULT]", "texture/basic_diff.tga");
    u32 spec = load_texture(&pack, "[TEX_SPEC_DEFAULT]", "texture/basic_spec.tga");
    load_material(&pack, "[MATERIAL_DEFAULT]", diff, spec, 0.5f);

    default_mesh(&pack);
    load_obj_file(&pack, "mesh/prim_sphere.ric");
    load_obj_file(&pack, "mesh/sphere.ric");
    load_obj_file(&pack, "mesh/wall_cornertest.ric");
    //load_obj_file(&pack, "mesh/conference.ric");
    //load_obj_file(&pack, "mesh/spawn.ric");
    //load_obj_file(&pack, "mesh/door.ric");
    //load_obj_file(&pack, "mesh/welcome_floor.ric");
    //load_obj_file(&pack, "mesh/grass.ric");

    pack.index_offset = sizeof(struct pack);
    pack.data_offset = pack.index_offset +
                       pack.blob_count * sizeof(struct pack_entry);

    // TODO: Write pack, index, data to disk.
    FILE *pack_file = fopen("packs/alpha.ric", "wb");
    if (pack_file)
    {
        fwrite(&pack, 1, sizeof(pack), pack_file);
        fwrite(pack.index, 1, pack.blob_count * sizeof(*pack.index), pack_file);
        fwrite(pack.buffer, 1, pack.buffer_used, pack_file);
        fclose(pack_file);
    }
    else
    {
        err = RICO_ERROR(ERR_FILE_WRITE, "Error: Cannot open pack file.\n");
        goto cleanup;
    }

    u32 tick_end = SDL_GetTicks();
    printf("[PERF][pack] Pack written in: %d ticks\n", tick_end - tick_start);

cleanup:
    free(pack.index);
    free(pack.buffer);
    return err;
}

int pack_load(const char *filename)
{
    UNUSED(filename);
    enum rico_error err = SUCCESS;
    //struct pack *pack; // = load_entire_file()
    // TODO: Load pack directly into memory from file, fix pointers using
    //       offsets.
    // NOTE: Fix pack.buffer_size to be w/e size is actually allocated
    return err;
}