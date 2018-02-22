#define MESH_VERTICES_MAX 200000

// TODO: Tune these as necessary
#define MAX_PACK_BLOBS 4096
#define MAX_PACK_BUF_SIZE MB(512)
#define DEFAULT_PACK_BLOBS 32
#define DEFAULT_PACK_BUF_SIZE KB(256)

static const u8 PACK_SIGNATURE[4] = { 'R', 'I', 'C', 'O' };
//internal const u32 PACK_SIGNATURE =
//    ((u32)'R' << 24) | ((u32)'I' << 16) | ((u32)'C' << 8) | ((u32)'O');

u32 RICO_packs_next = 0;
struct pack *RICO_packs[MAX_PACKS] = { 0 };

static void *blob_start(struct pack *pack, enum rico_hnd_type type, u32 size,
                          const char *name)
{
    RICO_ASSERT(pack->blob_current_id == 0);
    RICO_ASSERT(pack->blobs_used < pack->blob_count);

    if (!size) size = rico_hnd_type_size[type];

    // TODO: Free list instead of loop?
    // Find first available lookup id
    u32 id = 1;
    while (pack->lookup[id])
    {
        RICO_ASSERT(id < pack->blob_count);
        id++;
    }

    pack->blob_current_id = id;
    pack->lookup[pack->blob_current_id] = pack->blobs_used;
    pack->index[pack->blobs_used].type = type;
    pack->index[pack->blobs_used].offset = pack->buffer_used;
    pack->index[pack->blobs_used].size = 0;

    pkid pkid = PKID_GENERATE(pack->id, id);

    struct uid *uid = push_bytes(pack, size);
    uid->pkid = pkid;
    uid->type = type;
    memcpy(uid->name, name, MIN(sizeof(uid->name) - 1, dlb_strlen(name)));

    RICO_ASSERT(uid->type);
    return uid;
}
static u32 blob_offset(struct pack *pack)
{
	RICO_ASSERT(pack->blob_current_id);
	RICO_ASSERT(pack->index[pack->lookup[pack->blob_current_id]].offset <
				pack->buffer_used);

	u32 offset = pack->buffer_used -
		pack->index[pack->lookup[pack->blob_current_id]].offset;
	return offset;
}
static void blob_end(struct pack *pack)
{
	RICO_ASSERT(pack->blob_current_id);
	RICO_ASSERT(pack->lookup[pack->blob_current_id] == pack->blobs_used);
	u32 size = blob_offset(pack);

#if RICO_DEBUG_PACK
	static u32 max_alloc = 0;
	if (size > max_alloc)
	{
		max_alloc = size;
		printf("[PERF][Pack] New max alloc for '%s': %u bytes\n", pack->name,
			   max_alloc);
	}
#endif

	pack->index[pack->blobs_used].size = size;
	pack->blobs_used++;
	pack->blob_current_id = 0;

	// Compact buffer if over 50% utilized
	u32 half_blobs = pack->blobs_used >> 1;
	if (half_blobs > pack->blob_count - pack->blobs_used)
	{
		pack_compact_buffer(pack);
	}

	u32 half_buffer = pack->buffer_used >> 1;
	if (half_buffer > pack->buffer_size - pack->buffer_used)
	{
		pack_compact_buffer(pack);
	}
}
static void blob_error(struct pack *pack, u32 *pack_idx)
{
	pack_pop(pack, *pack_idx);
	*pack_idx = UINT_MAX;
}
static void null_blob(struct pack *pack)
{
    push_string(pack, "[This page intentionally left blank]");
    pack->index[0].size = pack->buffer_used;
    pack->blobs_used++;
}

void *RICO_pack_lookup(pkid pkid)
{
    u32 pack_id = PKID_PACK(pkid);
    RICO_ASSERT(pack_id < MAX_PACKS);
    struct pack *pack = RICO_packs[pack_id];

    u32 blob_id = PKID_BLOB(pkid);
    RICO_ASSERT(blob_id < pack->blob_count);
    return pack_read(pack, pack->lookup[blob_id]);
}

static void pack_expand(struct pack *pack)
{
    // TODO: Expand pack by min(size * 2, MAX_EXPAND)
    UNUSED(pack);
}

// NOTE: This will break all existing pointers
static void pack_compact_buffer(struct pack *pack)
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

static u32 perf_pack_tick_start;
static u32 perf_pack_tick_end;

struct pack *RICO_pack_init(u32 id, const char *name, u32 blob_count,
					        u32 buffer_size)
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
	if (RICO_packs_next <= pack->id)
	{
		RICO_packs_next = pack->id + 1;
		RICO_ASSERT(RICO_packs_next < MAX_PACKS);
	}
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

    RICO_packs[pack->id] = pack;
    return pack;
}

int RICO_pack_save(struct pack *pack, const char *filename, bool shrink)
{
    RICO_ASSERT(pack->blob_current_id == 0);
    enum rico_error err = SUCCESS;

    pack_compact_buffer(pack);

#if RICO_SAVE_BACKUP
	// Save backup copy
	time_t rawtime = time(NULL);
	struct tm *tm = localtime(&rawtime);

	char backupFilename[128] = { 0 };
	int len = snprintf(backupFilename, sizeof(backupFilename),
					   "chunks/cereal_%04d%02d%02dT%02d%02d%02d.bin",
					   1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday,
					   tm->tm_hour, tm->tm_min, tm->tm_sec);
	string_truncate(backupFilename, sizeof(backupFilename), len);

	struct rico_file backupFile;
	err = rico_file_open_write(&backupFile, backupFilename,
							   RICO_FILE_VERSION_CURRENT);
	if (err) return err;

	err = rico_serialize(chunk, &backupFile);
	rico_file_close(&backupFile);
	return err;
#endif

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
        bytes_written += (u32)fwrite(pack, 1, sizeof(*pack), pack_file);
        bytes_written += (u32)fwrite(pack->lookup, 1,
									 pack->blob_count * sizeof(pack->lookup[0]),
									 pack_file);
        bytes_written += (u32)fwrite(pack->index, 1,
									 pack->blob_count * sizeof(pack->index[0]),
									 pack_file);
        bytes_written += (u32)fwrite(pack->buffer, 1, pack->buffer_used,
									 pack_file);
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

int RICO_pack_load(const char *filename, struct pack **_pack)
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
        u32 bytes_read = (u32)fread(pack, 1, size, pack_file);
        fclose(pack_file);
        RICO_ASSERT(bytes_read == size_on_disk);
        RICO_ASSERT(pack->blob_current_id == 0); // If not zero, WTF??

        u8 *base = (u8 *)pack;
        pack->lookup = (void *)(base + pack->lookup_offset);
        pack->index = (void *)(base + pack->index_offset);
        pack->buffer = base + pack->data_offset;
        if (_pack) *_pack = pack;

        RICO_ASSERT(pack->id < MAX_PACKS);
        RICO_ASSERT(RICO_packs[pack->id] == 0);
        RICO_packs[pack->id] = pack;

        u32 tick_end = SDL_GetTicks();
        printf("[PERF][pack] '%s' loaded in: %d ticks\n", pack->name,
               tick_end - tick_start);
    }
    else
    {
        err = RICO_ERROR(ERR_FILE_READ,
                         "Error: Cannot open pack file [%s] for read.\n",
                         filename);
        goto cleanup;
    }

cleanup:
    return err;
}

void RICO_pack_free(u32 id)
{
    RICO_ASSERT(id < MAX_PACKS);
    RICO_ASSERT(RICO_packs[id]);
    free(RICO_packs[id]);
    RICO_packs[id] = 0;
}

static void *pack_next(pkid pkid, enum rico_hnd_type type)
{
    u32 pack_id = PKID_PACK(pkid);
    u32 blob_id = PKID_BLOB(pkid);

    struct pack *pack = RICO_packs[pack_id];
    RICO_ASSERT(pack);

    if (pack->blobs_used == 0)
        return NULL;

    u32 start_idx = (blob_id) ? pack->lookup[blob_id] : 1;
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

static void *pack_prev(pkid pkid, enum rico_hnd_type type)
{
    u32 pack_id = PKID_PACK(pkid);
    u32 blob_id = PKID_BLOB(pkid);

    struct pack *pack = RICO_packs[pack_id];
    RICO_ASSERT(pack);

    if (pack->blobs_used == 0)
        return NULL;

    u32 start_idx = (blob_id) ? pack->lookup[blob_id] : pack->blobs_used;
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

static void pack_delete(pkid pkid)
{
    u32 pack_id = PKID_PACK(pkid);
    u32 blob_id = PKID_BLOB(pkid);
    RICO_ASSERT(pack_id < MAX_PACKS);

    struct pack *pack = RICO_packs[pack_id];
    RICO_ASSERT(pack);
    RICO_ASSERT(blob_id < pack->blob_count);

    // HACK: For now, just ignore requests to delete blobs in the default pack
    if (pack_id == PACK_DEFAULT)
        return;

    // TODO: Why was I allowed blob_id == 0 to return? Is this something that's
    //       already been deleted?
    RICO_ASSERT(blob_id);
    if (blob_id == 0)
        return;

    RICO_ASSERT(blob_id < pack->blob_count);

    u32 index = pack->lookup[blob_id];
    RICO_ASSERT(index > 0);
    RICO_ASSERT(index < pack->blobs_used);

    enum rico_hnd_type type = pack->index[index].type;
    RICO_ASSERT(type);

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
			object_delete(obj);
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
			string_delete(string);
			break;
		}
		default:
			break;
    }
}
pkid RICO_load_object(struct pack *pack, u32 type, u32 size, const char *name)
{
    struct rico_object *obj = blob_start(pack, RICO_HND_OBJECT, size, name);
    obj->type = type;
    obj->xform.orientation = QUAT_IDENT;
    obj->xform.scale = VEC3_ONE;
    object_bbox_recalculate(obj);
    object_transform_update(obj);

    pkid pkid = obj->uid.pkid;
    blob_end(pack);
    return pkid;
}
pkid RICO_load_texture(struct pack *pack, const char *name, GLenum target,
                       u32 width, u32 height, u8 bpp, u8 *pixels)
{
    struct rico_texture *tex = blob_start(pack, RICO_HND_TEXTURE,
                                          sizeof(struct rico_texture), name);
    tex->width = width;
    tex->height = height;
    tex->bpp = bpp;
    tex->gl_target = target;

    tex->pixels_offset = blob_offset(pack);
    push_data(pack, pixels, tex->width * tex->height, tex->bpp / 8);

    pkid pkid = tex->uid.pkid;
    blob_end(pack);
    return pkid;
}
pkid RICO_load_texture_file(struct pack *pack, const char *name,
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

    pkid tex_pkid = RICO_load_texture(pack, name, GL_TEXTURE_2D, (u32)width,
                                 (u32)height, 32, pixels);

cleanup:
    stbi_image_free(pixels);
    if (err) blob_error(pack, &tex_pkid);
    return tex_pkid;
}
pkid RICO_load_texture_color(struct pack *pack, const char *name,
                             struct vec4 color)
{
    u8 rgba[4] = {
        (u8)(color.r * 255),
        (u8)(color.g * 255),
        (u8)(color.b * 255),
        (u8)(color.a * 255)
    };
    return RICO_load_texture(pack, name, GL_TEXTURE_2D, 1, 1, 32, rgba);
}
pkid RICO_load_material(struct pack *pack, const char *name, pkid tex_albedo,
                        pkid tex_mrao, pkid tex_emission)
{
    struct rico_material *mat = blob_start(pack, RICO_HND_MATERIAL, 0, name);
    mat->tex_albedo = tex_albedo;
    mat->tex_mrao = tex_mrao;
    mat->tex_emission = tex_emission;

    pkid pkid = mat->uid.pkid;
    blob_end(pack);
    return pkid;
}
pkid RICO_load_font_file(struct pack *pack, const char *name,
                         const char *filename)
{
    enum rico_error err;
    pkid font_pkid = 0;

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
    struct rico_font *font = blob_start(pack, RICO_HND_FONT, 0, name);
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

    font_pkid = font->uid.pkid;
    blob_end(pack);

    //------------------------------------------------------------
    // Font texture
    //------------------------------------------------------------
    struct rico_texture *tex0 = blob_start(pack, RICO_HND_TEXTURE, 0, name);
    tex0->width = tex_width;
    tex0->height = tex_height;
    tex0->bpp = tex_bpp;
    tex0->gl_target = GL_TEXTURE_2D;  // Fonts are always 2D textures

    tex0->pixels_offset = blob_offset(pack);
    push_data(pack, &buffer[MAP_DATA_OFFSET], tex_width * tex_height,
              tex_bpp / 8);

    pkid tex0_pkid = tex0->uid.pkid;
    blob_end(pack);

    font = RICO_pack_lookup(font_pkid);
    font->tex_pkid = tex0_pkid;

cleanup:
    free(buffer);
    return font_pkid;
}
pkid RICO_load_mesh(struct pack *pack, const char *name, u32 vertex_size,
                    u32 vertex_count, const void *vertex_data,
                    u32 element_count, const GLuint *element_data)
{
    struct rico_mesh *mesh = blob_start(pack, RICO_HND_MESH, 0, name);
    mesh->vertex_size = vertex_size;
    mesh->vertex_count = vertex_count;
    mesh->element_count = element_count;
    mesh->vertices_offset = blob_offset(pack);
    push_data(pack, vertex_data, vertex_count, vertex_size);
    mesh->elements_offset = blob_offset(pack);
    push_data(pack, element_data, element_count, sizeof(*element_data));

    pkid pkid = mesh->uid.pkid;
    blob_end(pack);
    return pkid;
}
pkid RICO_load_string(struct pack *pack, enum rico_string_slot slot, float x,
                      float y, struct vec4 color, u32 lifespan,
                      pkid font, const char *text)
{
#if RICO_DEBUG_STRING
    printf("[strg][init] name=%s\n", name);
#endif

    const char *name = rico_string_slot_string[slot];

    // TODO: Reuse mesh and material if they are the same
    // Generate font mesh and get texture handle
    u32 font_mesh_id;
    u32 font_tex_id;
    font_render(&font_mesh_id, &font_tex_id, font, x, y, color, text, name);

    pkid str_object_pkid =
        RICO_load_object(pack, RICO_OBJECT_TYPE_STRING_SCREEN, 0, name);
    struct rico_object *str_object = RICO_pack_lookup(str_object_pkid);
    str_object->mesh_pkid = font_mesh_id;

    struct rico_string *str = blob_start(pack, RICO_HND_STRING, 0, "bloop");
    str->slot = slot;
    str->object_id = str_object->uid.pkid;
    str->lifespan = lifespan;

    // Store in slot table if not dynamic
    if (str->slot != STR_SLOT_DYNAMIC)
    {
        hashtable_insert(&global_string_slots, &str->slot, sizeof(str->slot),
                         &str->uid.pkid, sizeof(str->uid.pkid));
    }

    pkid pkid = str->uid.pkid;
    blob_end(pack);
    return pkid;
}

int RICO_load_obj_file(struct pack *pack, const char *filename,
                       pkid *_last_mesh_id)
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
                last_mesh_id = RICO_load_mesh(pack, name, sizeof(*vertices),
                                         idx_vertex, vertices, idx_element,
                                         elements);
                struct rico_mesh *mesh = RICO_pack_lookup(last_mesh_id);
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
        last_mesh_id = RICO_load_mesh(pack, name, sizeof(*vertices), idx_vertex,
                                 vertices, idx_element, elements);
        struct rico_mesh *mesh = RICO_pack_lookup(last_mesh_id);
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

static void pack_build_default()
{
	// TODO: This entire pack could be embedded as binary data in the .exe once
	//       the contents are finalized. This would allow the engine to run even
	//       when the data directory is missing.
	const char *filename = "packs/default.pak";

#if 0
	FILE *default_pack = fopen(filename, "rb");
	if (default_pack)
	{
		fclose(default_pack);
		return;
	}
#endif

	struct pack *pack = RICO_pack_init(PACK_DEFAULT, filename, 16, MB(1));
    pkid font_id = RICO_load_font_file(pack, "[FONT_DEFAULT]",
                                       "font/cousine_regular.bff");
    pkid diff_id = RICO_load_texture_file(pack, "[TEX_DIFF_DEFAULT]",
                                          "texture/pbr_default_0.tga");
    pkid spec_id = RICO_load_texture_file(pack, "[TEX_SPEC_DEFAULT]",
                                          "texture/pbr_default_1.tga");
    pkid emis_id = RICO_load_texture_file(pack, "[TEX_EMIS_DEFAULT]",
                                          "texture/pbr_default_2.tga");
    pkid mat_id = RICO_load_material(pack, "[MATERIAL_DEFAULT]", diff_id,
                                     spec_id, emis_id);

	// HACK: This is a bit of a gross way to get the id of the last mesh
    pkid cube;
	RICO_load_obj_file(pack, "mesh/prim_cube.obj", &cube);
    pkid sphere;
	RICO_load_obj_file(pack, "mesh/prim_sphere.obj", &sphere);

    struct rico_font *font = RICO_pack_lookup(font_id);
	RICO_ASSERT(font_id == FONT_DEFAULT);
	RICO_ASSERT(font->tex_pkid == FONT_DEFAULT_TEXTURE);
	RICO_ASSERT(diff_id == TEXTURE_DEFAULT_DIFF);
	RICO_ASSERT(spec_id == TEXTURE_DEFAULT_SPEC);
	RICO_ASSERT(emis_id == TEXTURE_DEFAULT_EMIS);
	RICO_ASSERT(mat_id == MATERIAL_DEFAULT);
	RICO_ASSERT(cube == MESH_DEFAULT_CUBE);
	RICO_ASSERT(sphere == MESH_DEFAULT_SPHERE);

	RICO_pack_save(pack, filename, true);
	RICO_pack_free(pack->id);
}
