#define MESH_VERTICES_MAX 200000

// TODO: Tune these as necessary
#define MAX_PACK_BLOBS 4096
#define MAX_PACK_BUF_SIZE MB(512)
#define DEFAULT_PACK_BLOBS 32
#define DEFAULT_PACK_BUF_SIZE KB(256)

internal const u8 PACK_SIGNATURE[4] = { 'R', 'I', 'C', 'O' };
//internal const u32 PACK_SIGNATURE =
//    ((u32)'R' << 24) | ((u32)'I' << 16) | ((u32)'C' << 8) | ((u32)'O');

struct pack *packs[MAX_PACKS] = { 0 };

global void *pack_next(struct pack *pack, u32 id, enum rico_hnd_type type)
{
    if (pack->blobs_used == 0)
        return NULL;

    u32 start_idx = (id) ? pack->lookup[ID_BLOB(id)] : 1;
    u32 idx = start_idx;
    do
    {
        idx++;
        if (idx == pack->blobs_used)
            idx = 1;

        if (pack->index[idx].type == type)
        {
            return pack_read(pack, idx);
        }
    } while (idx != start_idx);

    return NULL;
}

global void *pack_prev(struct pack *pack, u32 id, enum rico_hnd_type type)
{
    if (pack->blobs_used == 0)
        return NULL;

    u32 start_idx = (id) ? pack->lookup[ID_BLOB(id)] : pack->blobs_used;
    u32 idx = start_idx;
    do
    {
        idx--;
        if (idx == 0)
            idx = pack->blobs_used - 1;

        if (pack->index[idx].type == type)
        {
            return pack_read(pack, idx);
        }
    } while (idx != start_idx);

    return NULL;
}

internal void pack_expand(struct pack *pack)
{
    // TODO: Expand pack by min(size * 2, MAX_EXPAND)
    UNUSED(pack);
}

// NOTE: This will break all existing pointers
internal void pack_compact_buffer(struct pack *pack)
{
    // Can't rearrange memory in the middle of creating a blob, because it will
    // break any pointers in the load() functions.
    RICO_ASSERT(pack->blob_current_id == 0);

#if RICO_DEBUG_PACK
    u32 perf_start = SDL_GetTicks();
#endif

    // Skip NULL blob
    u32 offset = pack->index[0].size;

    u32 index = 1;
    while (index < pack->blob_count && pack->index[index].type)
    {
        // Skip free blobs
        if (!pack->index[index].type)
            continue;

        // If there's room to compact, move this blob left
        if (pack->index[index].offset > offset)
        {
            memcpy(pack->buffer + offset,
                   pack->buffer + pack->index[index].offset,
                   pack->index[index].size);
            pack->index[index].offset = offset;
        }
        offset += pack->index[index].size;
        index++;
    }
    pack->buffer_used = offset;

    // HACK: Compact *must* zero the remaining memory in the buffer after it
    //       moves things because the load() functions don't set every member;
    //       they assume that they are given zeroed memory.
    memset(pack->buffer + pack->buffer_used, 0,
           pack->buffer_size - pack->buffer_used);

#if RICO_DEBUG_PACK
    u32 perf_end = SDL_GetTicks();
    printf("[PERF][Pack] Pack %s buffer compacted in %u ticks.\n", pack->name,
           perf_end - perf_start);
#endif

    // If pack buffer is more than 50% utilized, expand it
    if (pack->buffer_size - pack->buffer_used < pack->buffer_used)
    {
        pack_expand(pack);
#if RICO_DEBUG_PACK
        printf("[PERF][Pack] Pack %s buffer compacted.\n", pack->name);
#endif
    }
}

global void pack_delete(struct pack *pack, u32 id, enum rico_hnd_type type)
{
    u32 pack_id = ID_PACK(id);
    u32 blob_id = ID_BLOB(id);

    // HACK: For now, just ignore requests to delete blobs in the default pack
    if (pack_id == PACK_DEFAULT)
        return;

    RICO_ASSERT(blob_id);
    RICO_ASSERT(blob_id < pack->blob_count);
    RICO_ASSERT(pack_id);
    RICO_ASSERT(pack_id == pack->id);

    if (blob_id == 0)
        return;

    RICO_ASSERT(blob_id < pack->blob_count);

    u32 index = pack->lookup[blob_id];
    RICO_ASSERT(index > 0);
    RICO_ASSERT(index < pack->blobs_used);
    RICO_ASSERT(pack->index[index].type);
    RICO_ASSERT(pack->index[index].type == type);

    void *delete_me = pack_read(pack, index);
    
    // 7 used, delete 4
    // 0 1 2 3 4 5 6 7 8 9
    // a b c d - e f [blob = 4]
    // a b c d e e f [blob = 5]
    // a b c d e f f

    // Shift everything after the deleted blob index left
    for (u32 idx = index; idx < pack->blobs_used - 1; ++idx)
    {
        pack->index[idx] = pack->index[idx + 1];
    }
    pack->lookup[blob_id] = 0;
    pack->blobs_used--;
    pack->index[pack->blobs_used].type = 0;

    // Update lookup table
    for (u32 i = 1; i < pack->blob_count; ++i)
    {
        if (pack->lookup[i] > index)
            pack->lookup[i]--;
    }

    switch (type)
    {
    case RICO_HND_OBJECT:
    {
        struct rico_object *obj = delete_me;
        object_delete(pack, obj);
        break;
    }
    case RICO_HND_TEXTURE:
    {
        struct rico_texture *tex = delete_me;
        texture_delete(tex);
        break;
    }
    case RICO_HND_MESH:
    {
        struct rico_mesh *mesh = delete_me;
        mesh_delete(mesh);
        break;
    }
    case RICO_HND_STRING:
    {
        struct rico_string *string = delete_me;
        string_delete(pack, string);
        break;
    }
    default:
        break;
    }
}

global u32 load_object(struct pack *pack, const char *name,
                       enum rico_obj_type type, u32 prop_count,
                       struct obj_property *props, const struct bbox *bbox)
{
    u32 obj_id = blob_start(pack, RICO_HND_OBJECT);
    struct rico_object *obj = push_bytes(pack, sizeof(*obj));
    obj->id = obj_id;
    obj->type = type;
    obj->xform.trans = VEC3_ZERO;
    obj->xform.rot = VEC3_ZERO;
    if (type == OBJ_STRING_SCREEN)
        obj->xform.scale = VEC3_SCALE_ASPECT;
    else
        obj->xform.scale = VEC3_ONE;
    obj->xform.matrix = MAT4_IDENT;
    obj->xform.matrix_inverse = MAT4_IDENT;
    object_transform_update(obj);

    obj->name_offset = blob_offset(pack);
    push_string(pack, name);

    obj->props_offset = blob_offset(pack);
    obj->prop_count = prop_count;
    if (prop_count)
    {
        push_data(pack, props, prop_count, sizeof(props[0]));
    }

    if (bbox)
    {
        obj->bbox = *bbox;
    }
    else
    {
        struct rico_mesh *mesh;
        struct obj_property *mesh_prop = object_prop(obj, PROP_MESH_ID);
        if (mesh_prop && mesh_prop->mesh_id)
        {
            mesh = pack_lookup(pack, mesh_prop->mesh_id);
        }
        else
        {
            mesh = pack_lookup(packs[PACK_DEFAULT], MESH_DEFAULT_CUBE);
        }
        obj->bbox = mesh->bbox;
    }

    blob_end(pack);
    return obj_id;
}

global u32 load_texture(struct pack *pack, const char *name, GLenum target,
                        u32 width, u32 height, u8 bpp, u8 *pixels)
{
    u32 tex_id = blob_start(pack, RICO_HND_TEXTURE);
    struct rico_texture *tex = push_bytes(pack, sizeof(*tex));
    tex->id = tex_id;
    tex->width = width;
    tex->height = height;
    tex->bpp = bpp;
    tex->gl_target = target;

    tex->name_offset = blob_offset(pack);
    push_string(pack, name);

    tex->pixels_offset = blob_offset(pack);
    push_data(pack, pixels, tex->width * tex->height, tex->bpp / 8);

    blob_end(pack);
    return tex_id;
}
global u32 load_texture_file(struct pack *pack, const char *name,
                             const char *filename)
{
    enum rico_error err = SUCCESS;

    int width, height;
    u8 *pixels = stbi_load(filename, &width, &height, NULL, 4);
    if (!pixels)
    {
        err = RICO_ERROR(ERR_TEXTURE_LOAD, "Failed to load texture file: %s",
                         filename);
        goto cleanup;
    }

    u32 tex_id = load_texture(pack, name, GL_TEXTURE_2D, (u32)width,
                              (u32)height, 32, pixels);

cleanup:
    stbi_image_free(pixels);
    if (err) blob_error(pack, &tex_id);
    return tex_id;
}
global u32 load_texture_color(struct pack *pack, const char *name,
                              struct vec4 color)
{
    u8 rgba[4] = {
        (u8)(color.r * 255),
        (u8)(color.g * 255),
        (u8)(color.b * 255),
        (u8)(color.a * 255)
    };
    return load_texture(pack, name, GL_TEXTURE_2D, 1, 1, 32, rgba);
}
global u32 load_material(struct pack *pack, const char *name, u32 tex0,
                         u32 tex1, u32 tex2)
{
    u32 mat_id = blob_start(pack, RICO_HND_MATERIAL);
    struct rico_material *mat = push_bytes(pack, sizeof(*mat));

    mat->id = mat_id;
    mat->tex_id[0] = tex0;
    mat->tex_id[1] = tex1;
    mat->tex_id[2] = tex2;

    mat->name_offset = blob_offset(pack);
    push_string(pack, name);

    blob_end(pack);
    return mat_id;
}
global u32 load_font(struct pack *pack, const char *name,
                     const char *filename, u32 *font_tex)
{
    enum rico_error err;

    // Read font file
    char *buffer = NULL;
    u32 length;
    err = file_contents(filename, &length, &buffer);
    if (err) goto cleanup;

    // Check file signature
    if ((u8)buffer[0] != 0xBF || (u8)buffer[1] != 0xF2)
    {
        err = RICO_ERROR(ERR_FILE_SIGNATURE, "Unexpected file signature");
        goto cleanup;
    }

    // Allocate font/texture
    u32 font_id = blob_start(pack, RICO_HND_FONT);
    struct rico_font *font = push_bytes(pack, sizeof(*font));
    font->id = font_id;

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

#if RICO_DEBUG
    // Check filesize
    u32 pixels_size = tex_width * tex_height * (tex_bpp / 8);
    RICO_ASSERT(MAP_DATA_OFFSET + pixels_size == length);
#endif

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
    // Font texture
    //------------------------------------------------------------
    u32 tex_id = blob_start(pack, RICO_HND_TEXTURE);
    struct rico_texture *tex0 = push_bytes(pack, sizeof(*tex0));
    tex0->id = tex_id;
    tex0->width = tex_width;
    tex0->height = tex_height;
    tex0->bpp = tex_bpp;
    tex0->gl_target = GL_TEXTURE_2D;  // Fonts are always 2D textures

    tex0->name_offset = blob_offset(pack);
    push_string(pack, name);

    tex0->pixels_offset = blob_offset(pack);
    push_data(pack, &buffer[MAP_DATA_OFFSET], tex_width * tex_height,
              tex_bpp / 8);

    blob_end(pack);

    font->tex_id = tex_id;
    *font_tex = font->tex_id;

cleanup:
    free(buffer);
    if (err) blob_error(pack, &font_id);
    return font_id;
}
global u32 load_mesh(struct pack *pack, const char *name, u32 vertex_size,
                     u32 vertex_count, const void *vertex_data,
                     u32 element_count, const GLuint *element_data)
{
    u32 mesh_id = blob_start(pack, RICO_HND_MESH);
    struct rico_mesh *mesh = push_bytes(pack, sizeof(*mesh));

    mesh->id = mesh_id;
    mesh->vertex_size = vertex_size;
    mesh->vertex_count = vertex_count;
    mesh->element_count = element_count;

    mesh->name_offset = blob_offset(pack);
    push_string(pack, name);
    mesh->vertices_offset = blob_offset(pack);
    push_data(pack, vertex_data, vertex_count, vertex_size);
    mesh->elements_offset = blob_offset(pack);
    push_data(pack, element_data, element_count, sizeof(*element_data));
    
    blob_end(pack);
    return mesh_id;
}
global u32 load_string(struct pack *pack, const char *name,
                       enum rico_string_slot slot, float x, float y,
                       struct vec4 color, u32 lifespan, struct rico_font *font,
                       const char *text)
{
#if RICO_DEBUG_STRING
    printf("[strg][init] name=%s\n", name);
#endif

    // TODO: Reuse mesh and material if they are the same
    // Generate font mesh and get texture handle
    u32 font_mesh_id;
    u32 font_tex_id;
    font_render(&font_mesh_id, &font_tex_id, font, 0, 0, color, text, name);

    ////////////////////////////////////////////////////////////////////////////

    u32 str_object_id;
    struct obj_property str_props[2] = { 0 };
    str_props[0].type = PROP_MESH_ID;
    str_props[0].mesh_id = font_mesh_id;
    str_props[1].type = PROP_TEXTURE_ID;
    str_props[1].texture_id = font_tex_id;
    str_object_id = load_object(pack, name, OBJ_STRING_SCREEN,
                                array_count(str_props), str_props, NULL);
    struct rico_object *str_obj = pack_lookup(pack, str_object_id);
    object_trans_set(str_obj, &VEC3(SCREEN_X(x), SCREEN_Y(y), -1.0f));

    ////////////////////////////////////////////////////////////////////////////

    u32 str_id = blob_start(pack, RICO_HND_STRING);
    struct rico_string *str = push_bytes(pack, sizeof(*str));
    str->id = str_id;
    str->slot = slot;
    str->object_id = str_object_id;
    str->lifespan = lifespan;

    // Store in slot table if not dynamic
    if (str->slot != STR_SLOT_DYNAMIC)
    {
        hashtable_insert(&global_string_slots, &str->slot,
                         sizeof(str->slot), &str->id, sizeof(str->id));
    }

    blob_end(pack);
    return str_id;
}

/*
internal u32 default_mesh(struct pack *pack, const char *name)
{
    //--------------------------------------------------------------------------
    // Create default mesh (rainbow cube)
    //--------------------------------------------------------------------------
    
    struct vec4 color = VEC4(1.0f, 0.0f, 0.0f, 0.2f);
    const struct rico_vertex verts[] = {
        {
            VEC3(-1.0f, -1.0f, 1.0f),
            color,
            VEC3(-1.0f, -1.0f, 1.0f),
            VEC2(0.0f, 0.0f)
        },
        {
            VEC3(1.0f, -1.0f, 1.0f),
            color,
            VEC3(1.0f, -1.0f, 1.0f),
            VEC2(1.0f, 0.0f)
        },
        {
            VEC3(1.0f, 1.0f, 1.0f),
            color,
            VEC3(1.0f, 1.0f, 1.0f),
            VEC2(1.0f, 1.0f)
        },
        {
            VEC3(-1.0f, 1.0f, 1.0f),
            color,
            VEC3(-1.0f, 1.0f, 1.0f),
            VEC2(0.0f, 1.0f)
        },
        {
            VEC3(-1.0f, -1.0f, -1.0f),
            color,
            VEC3(-1.0f, -1.0f, -1.0f),
            VEC2(0.0f, 0.0f)
        },
        {
            VEC3(1.0f, -1.0f, -1.0f),
            color,
            VEC3(1.0f, -1.0f, -1.0f),
            VEC2(1.0f, 0.0f)
        },
        {
            VEC3(1.0f, 1.0f, -1.0f),
            color,
            VEC3(1.0f, 1.0f, -1.0f),
            VEC2(1.0f, 1.0f)
        },
        {
            VEC3(-1.0f, 1.0f, -1.0f),
            color,
            VEC3(-1.0f, 1.0f, -1.0f),
            VEC2(0.0f, 1.0f)
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
*/

int load_obj_file(struct pack *pack, const char *filename, u32 *_last_mesh_id)
{
    enum rico_error err;
    u32 last_mesh_id = 0;

    // TODO: Colossal waste of memory, just preprocess the file and count them!
    struct vec3 *positions = calloc(MESH_VERTICES_MAX, sizeof(*positions));
    struct vec2 *texcoords = calloc(MESH_VERTICES_MAX, sizeof(*texcoords));
    struct vec3 *normals = calloc(MESH_VERTICES_MAX, sizeof(*normals));
    struct pbr_vertex *vertices = calloc(MESH_VERTICES_MAX, sizeof(*vertices));
    GLuint *elements = calloc(MESH_VERTICES_MAX, sizeof(*elements));

    u32 idx_pos = 0;
    u32 idx_tex = 0;
    u32 idx_normal = 0;
    u32 idx_vertex = 0;
    u32 idx_element = 0;
    u32 idx_mesh = 0;

    long vert_pos = 0;
    long vert_tex = 0;
    long vert_norm = 0;

    u32 length;
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
                last_mesh_id = load_mesh(pack, name, sizeof(*vertices),
                                         idx_vertex, vertices, idx_element,
                                         elements);
                struct rico_mesh *mesh = pack_lookup(pack, last_mesh_id);
                bbox_init_mesh(&mesh->bbox, mesh);

                idx_mesh++;
                if (idx_mesh > 10)
                    goto cleanup;
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
        last_mesh_id = load_mesh(pack, name, sizeof(*vertices), idx_vertex,
                                 vertices, idx_element, elements);
        struct rico_mesh *mesh = pack_lookup(pack, last_mesh_id);
        bbox_init_mesh(&mesh->bbox, mesh);

        idx_mesh++;
    }

cleanup:
    if (_last_mesh_id)
        *_last_mesh_id = last_mesh_id;

    if (buffer)     free(buffer);
    if (positions)  free(positions);
    if (texcoords)  free(texcoords);
    if (normals)    free(normals);
    if (vertices)   free(vertices);
    if (elements)   free(elements);
    return err;
}

internal void null_blob(struct pack *pack)
{
    push_string(pack, "[This page intentionally left blank]");
    pack->index[0].size = pack->buffer_used;
    pack->blobs_used++;
}

internal u32 perf_pack_tick_start;
internal u32 perf_pack_tick_end;
static u32 next_pack_id = PACK_COUNT;

struct pack *pack_init(u32 id, const char *name, u32 blob_count, u32 buffer_size)
{
    if (!blob_count) blob_count = DEFAULT_PACK_BLOBS;
    if (!buffer_size) buffer_size = DEFAULT_PACK_BUF_SIZE;
    RICO_ASSERT(blob_count < MAX_PACK_BLOBS);
    RICO_ASSERT(buffer_size < MAX_PACK_BUF_SIZE);
    RICO_ASSERT(id < MAX_PACKS);

    perf_pack_tick_start = SDL_GetTicks();

    struct pack *pack = calloc(1,
        sizeof(*pack) +
        blob_count * sizeof(pack->lookup[0]) +
        blob_count * sizeof(pack->index[0]) +
        buffer_size);
    
    pack->id = id;
    memcpy(pack->magic, PACK_SIGNATURE, sizeof(pack->magic));
    pack->version = RICO_FILE_VERSION_CURRENT;
    strncpy(pack->name, name, (u32)sizeof(pack->name) - 1);
    pack->blob_current_id = 0;
    pack->blob_count = blob_count;
    pack->blobs_used = 0;
    pack->buffer_size = buffer_size;
    pack->buffer_used = 0;
    pack->lookup_offset = sizeof(*pack);
    pack->index_offset = pack->lookup_offset +
                         pack->blob_count * sizeof(pack->lookup[0]);
    pack->data_offset = pack->index_offset +
                        pack->blob_count * sizeof(pack->index[0]);

    u8 *base = (u8 *)pack;
    pack->lookup = (void *)(base + pack->lookup_offset);
    pack->index = (void *)(base + pack->index_offset);
    pack->buffer = base + pack->data_offset;

    // First blob in pack must be the null blob
    null_blob(pack);

    packs[pack->id] = pack;
    return pack;
}

int pack_save(struct pack *pack, const char *filename, bool shrink)
{
    RICO_ASSERT(pack->blob_current_id == 0);
    enum rico_error err = SUCCESS;

    pack_compact_buffer(pack);

    // Write pack file to disk
    FILE *pack_file = fopen(filename, "wb");
    if (pack_file)
    {
        u32 buffer_size = pack->buffer_size;

        if (shrink)
        {
            pack->buffer_size = pack->buffer_used;
        }

        u32 pack_size = sizeof(*pack) +
                        pack->blob_count * sizeof(pack->lookup[0]) +
                        pack->blob_count * sizeof(pack->index[0]) +
                        pack->buffer_used;

        u32 bytes_written = 0;
        bytes_written += fwrite(pack, 1, sizeof(*pack), pack_file);
        bytes_written += fwrite(pack->lookup, 1,
                                pack->blob_count * sizeof(pack->lookup[0]),
                                pack_file);
        bytes_written += fwrite(pack->index, 1,
                                pack->blob_count * sizeof(pack->index[0]),
                                pack_file);
        bytes_written += fwrite(pack->buffer, 1, pack->buffer_used, pack_file);
        fclose(pack_file);
        RICO_ASSERT(bytes_written == pack_size);

        if (shrink)
        {
            // Restore actual blob count / buffer size in memory
            pack->buffer_size = buffer_size;
        }
    }
    else
    {
        err = RICO_ERROR(ERR_FILE_WRITE,
                         "Error: Cannot open pack file [%s] for write.\n",
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

    // Read pack file from disk
    FILE *pack_file = fopen(filename, "rb");
    if (pack_file)
    {
        u32 tick_start = SDL_GetTicks();

        struct pack tmp_pack = { 0 };
        fread(&tmp_pack, 1, sizeof(tmp_pack), pack_file);
        rewind(pack_file);

        u32 size = tmp_pack.data_offset + tmp_pack.buffer_size;
        u32 size_on_disk = tmp_pack.data_offset + tmp_pack.buffer_used;

        struct pack *pack = calloc(1, size);
        u32 bytes_read = fread(pack, 1, size, pack_file);
        fclose(pack_file);
        RICO_ASSERT(bytes_read == size_on_disk);
        RICO_ASSERT(pack->blob_current_id == 0); // If not zero, WTF??

        u8 *base = (u8 *)pack;
        pack->lookup = (void *)(base + pack->lookup_offset);
        pack->index = (void *)(base + pack->index_offset);
        pack->buffer = base + pack->data_offset;
        if (_pack) *_pack = pack;

        RICO_ASSERT(pack->id < MAX_PACKS);
        RICO_ASSERT(packs[pack->id] == 0);
        packs[pack->id] = pack;
        if (pack->id >= next_pack_id)
        {
            next_pack_id = pack->id + 1;
        }

        u32 tick_end = SDL_GetTicks();
        printf("[PERF][pack] '%s' loaded in: %d ticks\n", pack->name,
               tick_end - tick_start);
    }
    else
    {
        err = RICO_ERROR(ERR_FILE_WRITE,
                         "Error: Cannot open pack file [%s] for write.\n",
                         filename);
        goto cleanup;
    }

cleanup:
    return err;
}

void pack_free(u32 id)
{
    RICO_ASSERT(id < MAX_PACKS);
    RICO_ASSERT(packs[id]);
    free(packs[id]);
    packs[id] = 0;
}

void pack_build_default(u32 id)
{
    // TODO: This entire pack could be embedded as binary data in the .exe once
    //       the contents are finalized. This would allow the engine to run even
    //       when the data directory is missing.
    const char *filename = "packs/default.pak";

    struct pack *pack = pack_init(id, filename, 16, MB(1));
    u32 font_tex = 0;
    u32 font = load_font(pack, "[FONT_DEFAULT]", "font/cousine_regular.bff",
                         &font_tex);
    u32 diff = load_texture_file(pack, "[TEX_DIFF_DEFAULT]",
                                 "texture/pbr_default_0.tga");
    u32 spec = load_texture_file(pack, "[TEX_SPEC_DEFAULT]",
                                 "texture/pbr_default_1.tga");
    u32 emis = load_texture_file(pack, "[TEX_EMIS_DEFAULT]",
                                 "texture/pbr_default_2.tga");
    u32 mat  = load_material(pack, "[MATERIAL_DEFAULT]", diff, spec, emis);

    // HACK: This is a bit of a gross way to get the id of the last mesh
    u32 cube;
    load_obj_file(pack, "mesh/prim_cube.obj", &cube);
    u32 sphere;
    load_obj_file(pack, "mesh/prim_sphere.obj", &sphere);

    RICO_ASSERT(font == FONT_DEFAULT);
    RICO_ASSERT(font_tex == FONT_DEFAULT_TEXTURE);
    RICO_ASSERT(diff == TEXTURE_DEFAULT_DIFF);
    RICO_ASSERT(spec == TEXTURE_DEFAULT_SPEC);
    RICO_ASSERT(emis == TEXTURE_DEFAULT_EMIS);
    RICO_ASSERT(mat == MATERIAL_DEFAULT);
    RICO_ASSERT(cube == MESH_DEFAULT_CUBE);
    RICO_ASSERT(sphere == MESH_DEFAULT_SPHERE);

    pack_save(pack, filename, true);
    pack_free(pack->id);
}

void pack_build_alpha(u32 id)
{
    // TODO: Split objects (state) from resources (materials, textures, audio).
    //       pack_state: all world objects (for now)
    //       pack_alpha: textures, materials, audio, etc. specific to alpha
    const char *filename = "packs/alpha.pak";

    struct pack *pack = pack_init(id, filename, 128, MB(256));
    u32 bricks_diff = load_texture_file(pack, "Bricks_diff", "texture/cobble_diff.tga");
    u32 bricks_mrao = load_texture_file(pack, "Bricks_mrao", "texture/cobble_mrao.tga");
    u32 bricks_emis = load_texture_color(pack, "Bricks_emis", COLOR_TRANSPARENT);
    u32 bricks_mat = load_material(pack, "Bricks", bricks_diff, bricks_mrao, bricks_emis);

    u32 door_mesh, ground_mesh;
    load_obj_file(pack, "mesh/alpha_door_001.obj", &door_mesh);
    load_obj_file(pack, "mesh/alpha_staircase_001.obj", 0);
    load_obj_file(pack, "mesh/alpha_wall_001.obj", 0);
    load_obj_file(pack, "mesh/alpha_terrain_001.obj", &ground_mesh);
    load_obj_file(pack, "mesh/alpha_game_panel.obj", 0);
    load_obj_file(pack, "mesh/alpha_game_button.obj", 0);

    struct obj_property ground_props[2] = { 0 };
    ground_props[0].type = PROP_MESH_ID;
    ground_props[0].material_id = ground_mesh;
    ground_props[1].type = PROP_MATERIAL_ID;
    ground_props[1].material_id = bricks_mat;
    load_object(pack, "Ground", OBJ_TERRAIN, array_count(ground_props),
                ground_props, NULL);

    u32 timmy_mat = load_material(pack, "Timmy", 0, 0, 0);
    struct obj_property timmy_props[4] = { 0 };
    timmy_props[0].type = PROP_MESH_ID;
    timmy_props[0].mesh_id = door_mesh;
    timmy_props[1].type = PROP_MATERIAL_ID;
    timmy_props[1].material_id = timmy_mat;
    timmy_props[2].type = PROP_LIGHT_SWITCH;
    timmy_props[2].light_switch = (struct light_switch) { 3, true };
    timmy_props[3].type = PROP_AUDIO_SWITCH;
    timmy_props[3].audio_switch = (struct audio_switch) { 3, true };
    load_object(pack, "Timmy", OBJ_STATIC, array_count(timmy_props),
                timmy_props, NULL);

    pack_save(pack, filename, false);
    pack_free(pack->id);
}

void pack_build_all()
{
    pack_build_default(PACK_DEFAULT);
    pack_build_alpha(next_pack_id);
    next_pack_id++;
}

int pack_load_all()
{
    enum rico_error err;

    err = pack_load("packs/default.pak", 0);
    pack_init(PACK_TRANSIENT, "pack_transient", 512, MB(4));
    pack_init(PACK_FRAME, "pack_frame", 0, 0);
    err = pack_load("packs/alpha.pak", &pack_active);

    for (u32 i = 0; i < array_count(packs); ++i)
    {
        if (packs[i])
            RICO_ASSERT(i == packs[i]->id);
    }

    return err;
}