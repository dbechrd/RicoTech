#define MESH_VERTICES_MAX 200000

// TODO: Tune these as necessary
#define MAX_PACK_BLOBS 4096
#define MAX_PACK_BUF_SIZE MB(512)
#define DEFAULT_PACK_BLOBS 32
#define DEFAULT_PACK_BUF_SIZE KB(256)

static const u8 PACK_SIGNATURE[4] = { 'R', 'I', 'C', 'O' };
//internal const u32 PACK_SIGNATURE =
//    ((u32)'R' << 24) | ((u32)'I' << 16) | ((u32)'C' << 8) | ((u32)'O');

struct pack *packs[MAX_PACKS] = { 0 };
u32 packs_next = 0;
u32 RICO_pack_active = 0;

static u32 perf_pack_tick_start;
static u32 perf_pack_tick_end;

static void pack_compact_buffer(struct pack *pack);

static void *blob_start(struct pack *pack, enum RICO_hnd_type type, u32 size,
                        const char *name)
{
    RICO_ASSERT(pack->blob_current_id == 0);
    RICO_ASSERT(pack->blobs_used < pack->blob_count);

    if (!size) size = RICO_hnd_type_size[type];

    // TODO: Free list instead of loop?
    // Find first available lookup id
    u32 id = 1;
    while (pack->lookup[id])
    {
        RICO_ASSERT(id < pack->blob_count);
        id++;
    }

    u32 name_hash = 0;
    MurmurHash3_x86_32(name, dlb_strlen(name), &name_hash);

    pack->blob_current_id = id;
    pack->lookup[pack->blob_current_id] = pack->blobs_used;
    pack->index[pack->blobs_used].type = type;
    pack->index[pack->blobs_used].name_hash = name_hash;
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

static void pack_expand(struct pack *pack)
{
    // TODO: Expand pack by min(size * 2, MAX_EXPAND)
    UNUSED(pack);
}
static void pack_compact_buffer(struct pack *pack)
{
    // NOTE: This will break all existing pointers

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
static void pack_delete(pkid id)
{
    u32 pack_id = PKID_PACK(id);
    u32 blob_id = PKID_BLOB(id);
    RICO_ASSERT(pack_id < MAX_PACKS);

    struct pack *pack = packs[pack_id];
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

    enum RICO_hnd_type type = pack->index[index].type;
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
            struct RICO_object *obj = delete_me;
            object_delete(obj);
            break;
        }
        case RICO_HND_TEXTURE:
        {
            struct RICO_texture *tex = delete_me;
            texture_delete(tex);
            break;
        }
        case RICO_HND_MESH:
        {
            struct RICO_mesh *mesh = delete_me;
            mesh_delete(mesh);
            break;
        }
        case RICO_HND_STRING:
        {
            struct RICO_string *string = delete_me;
            string_delete(string);
            break;
        }
        default:
            break;
    }
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

    u32 pack = RICO_pack_init(PACK_DEFAULT, filename, 16, MB(1));
    RICO_ASSERT(pack == 0); // PACK_DEFAULT *must* be id 0!

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

    struct RICO_font *font = RICO_pack_lookup(font_id);
    RICO_ASSERT(font_id == FONT_DEFAULT);
    RICO_ASSERT(font->tex_id == FONT_DEFAULT_TEXTURE);
    RICO_ASSERT(diff_id == TEXTURE_DEFAULT_DIFF);
    RICO_ASSERT(spec_id == TEXTURE_DEFAULT_SPEC);
    RICO_ASSERT(emis_id == TEXTURE_DEFAULT_EMIS);
    RICO_ASSERT(mat_id == MATERIAL_DEFAULT);
    RICO_ASSERT(cube == MESH_DEFAULT_CUBE);
    RICO_ASSERT(sphere == MESH_DEFAULT_SPHERE);

    RICO_pack_save(pack, true);
    RICO_pack_free(pack);
}

extern void *RICO_pack_lookup(pkid id)
{
    u32 pack_id = PKID_PACK(id);
    RICO_ASSERT(pack_id < MAX_PACKS);
    struct pack *pack = packs[pack_id];

    u32 blob_id = PKID_BLOB(id);
    RICO_ASSERT(blob_id < pack->blob_count);

    u32 idx = pack->lookup[blob_id];
    return pack_read(pack, idx);
}
extern void *RICO_pack_lookup_by_name(u32 pack_id, const char *name)
{
    RICO_ASSERT(pack_id < MAX_PACKS);
    struct pack *pack = packs[pack_id];

    u32 name_hash;
    MurmurHash3_x86_32(name, dlb_strlen(name), &name_hash);

    u32 idx = 0;
    while (idx < pack->blobs_used)
    {
        if (pack->index[idx].name_hash == name_hash)
        {
            return pack_read(pack, idx);
        }
        idx++;
    }

    return NULL;
}
extern pkid RICO_pack_first(u32 pack_id)
{
    struct pack *pack = packs[pack_id];
    RICO_ASSERT(pack);

    if (pack->blobs_used <= 1)
        return 0;

    struct uid *first = pack_read(pack, 1);
    return first->pkid;
}
extern pkid RICO_pack_last(u32 pack_id)
{
    struct pack *pack = packs[pack_id];
    RICO_ASSERT(pack);

    if (pack->blobs_used <= 1)
        return 0;

    struct uid *last = pack_read(pack, pack->blobs_used - 1);
    return last->pkid;
}
extern pkid RICO_pack_next(pkid id)
{
    RICO_ASSERT(id);
    u32 pack_id = PKID_PACK(id);
    u32 blob_id = PKID_BLOB(id);

    struct pack *pack = packs[pack_id];
    RICO_ASSERT(pack);
    RICO_ASSERT(blob_id < pack->blobs_used);

    if (pack->blobs_used <= 1)
        return 0;

    pkid next = 0;
    u32 idx = pack->lookup[blob_id] + 1;
    if (idx < pack->blobs_used)
    {
        struct uid *uid = pack_read(pack, idx);
        next = uid ? uid->pkid : 0;
    }
    return next;
}
extern pkid RICO_pack_prev(pkid id)
{
    RICO_ASSERT(id);
    u32 pack_id = PKID_PACK(id);
    u32 blob_id = PKID_BLOB(id);

    struct pack *pack = packs[pack_id];
    RICO_ASSERT(pack);
    RICO_ASSERT(blob_id < pack->blobs_used);

    if (pack->blobs_used <= 1)
        return 0;

    pkid prev = 0;
    u32 idx = pack->lookup[blob_id];
    if (idx > 1)
    {
        struct uid *uid = pack_read(pack, idx - 1);
        prev = uid ? uid->pkid : 0;
    }
    return prev;
}
extern pkid RICO_pack_next_loop(pkid id)
{
    pkid next = RICO_pack_next(id);
    if (!next)
    {
        next = RICO_pack_first(PKID_PACK(id));
    }
    return next;
}
extern pkid RICO_pack_prev_loop(pkid id)
{
    pkid prev = RICO_pack_prev(id);
    if (!prev)
    {
        prev = RICO_pack_last(PKID_PACK(id));
    }
    return prev;
}
extern pkid RICO_pack_first_type(u32 pack_id, enum RICO_hnd_type type)
{
    struct pack *pack = packs[pack_id];
    RICO_ASSERT(pack);

    if (pack->blobs_used <= 1)
        return 0;

    pkid first = 0;
    for (u32 idx = 1; idx < pack->blobs_used; ++idx)
    {
        if (pack->index[idx].type == type)
        {
            struct uid *uid = pack_read(pack, idx);
            first = uid->pkid;
            break;
        }
    }
    return first;
}
extern pkid RICO_pack_last_type(u32 pack_id, enum RICO_hnd_type type)
{
    struct pack *pack = packs[pack_id];
    RICO_ASSERT(pack);

    if (pack->blobs_used == 0)
        return 0;

    pkid last = 0;
    for (u32 idx = pack->blobs_used - 1; idx > 0; --idx)
    {
        if (pack->index[idx].type == type)
        {
            struct uid *uid = pack_read(pack, idx);
            last = uid->pkid;
            break;
        }
    }
    return last;
}
extern pkid RICO_pack_next_type(pkid id, enum RICO_hnd_type type)
{
    pkid next = RICO_pack_next(id);
    while (next)
    {
        struct uid *uid = RICO_pack_lookup(next);
        if (uid->type == type)
            break;

        next = RICO_pack_next(id);
    }
    return next;
}
extern pkid RICO_pack_prev_type(pkid id, enum RICO_hnd_type type)
{
    pkid prev = RICO_pack_prev(id);
    while (prev)
    {
        struct uid *uid = RICO_pack_lookup(prev);
        if (uid->type == type)
            break;

        prev = RICO_pack_prev(id);
    }
    return prev;
}
extern u32 RICO_pack_init(u32 pack_id, const char *name, u32 blob_count,
                          u32 buffer_size)
{
    // HACK: Don't pass id into init().. this was just to make sure packs get
    //       loaded back into the same id they were saved as. Surely there's a
    //       better way to meet that requirement than forcing the caller to
    //       always specify the pack id? That functionality should be internal!
    if (!blob_count) blob_count = DEFAULT_PACK_BLOBS;
    if (!buffer_size) buffer_size = DEFAULT_PACK_BUF_SIZE;
    RICO_ASSERT(blob_count < MAX_PACK_BLOBS);
    RICO_ASSERT(buffer_size < MAX_PACK_BUF_SIZE);
    RICO_ASSERT(pack_id < MAX_PACKS);

    u32 new_id = pack_id ? pack_id : packs_next;
    RICO_ASSERT(!packs[new_id]);

    // TODO: Use perf timer, not ticks. Expose RICO_timer somehow.
    perf_pack_tick_start = SDL_GetTicks();

    struct pack *pack = calloc(1, sizeof(*pack) +
                               blob_count * sizeof(pack->lookup[0]) +
                               blob_count * sizeof(pack->index[0]) +
                               buffer_size);

    pack->id = new_id;
	if (packs_next <= pack->id)
	{
		packs_next = pack->id + 1;
		RICO_ASSERT(packs_next < MAX_PACKS);
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

    packs[pack->id] = pack;
    return pack->id;
}
extern int RICO_pack_save(u32 pack_id, bool shrink)
{
    struct pack *pack = packs[pack_id];
    RICO_ASSERT(pack->blob_current_id == 0);

    return RICO_pack_save_as(pack_id, pack->name, shrink);
}
extern int RICO_pack_save_as(u32 pack_id, const char *filename, bool shrink)
{
    enum RICO_error err = SUCCESS;

    struct pack *pack = packs[pack_id];
    RICO_ASSERT(pack->blob_current_id == 0);

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
            // Restore actual blob bucket_count / buffer size in memory
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
extern int RICO_pack_load(const char *filename, u32 *_pack)
{
    enum RICO_error err = SUCCESS;

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
        if (_pack) *_pack = pack->id;

        RICO_ASSERT(pack->id < MAX_PACKS);
        RICO_ASSERT(packs[pack->id] == 0);
        packs[pack->id] = pack;

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
extern void RICO_pack_free(u32 pack_id)
{
    RICO_ASSERT(pack_id < MAX_PACKS);
    RICO_ASSERT(packs[pack_id]);
    free(packs[pack_id]);
    packs[pack_id] = 0;
}
extern pkid RICO_load_object(u32 pack_id, u32 type, u32 size, const char *name)
{
    struct pack *pack = packs[pack_id];
    struct RICO_object *obj = blob_start(pack, RICO_HND_OBJECT, size, name);
    obj->type = type;
    obj->xform.position = VEC3_ZERO;
    obj->xform.orientation = QUAT_IDENT;
    obj->xform.scale = VEC3_ONE;
    obj->mesh_id = 0;
    obj->material_id = 0;

    object_transform_update(obj);

    pkid pkid = obj->uid.pkid;
    blob_end(pack);
    return pkid;
}
extern pkid RICO_load_texture(u32 pack_id, const char *name, GLenum target,
                              u32 width, u32 height, u8 bpp, u8 *pixels)
{
    struct pack *pack = packs[pack_id];

    struct RICO_texture *tex = blob_start(pack, RICO_HND_TEXTURE,
                                          sizeof(struct RICO_texture), name);
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
extern pkid RICO_load_texture_file(u32 pack_id, const char *name,
                                   const char *filename)
{
    enum RICO_error err = SUCCESS;

    int width, height;
    u8 *pixels = stbi_load(filename, &width, &height, NULL, 4);
    if (!pixels)
    {
        err = RICO_ERROR(ERR_TEXTURE_LOAD, "Failed to load texture file: %s",
                         filename);
        goto cleanup;
    }

    pkid tex_id = RICO_load_texture(pack_id, name, GL_TEXTURE_2D, (u32)width,
                                    (u32)height, 32, pixels);

cleanup:
    stbi_image_free(pixels);
    if (err) blob_error(packs[pack_id], &tex_id);
    return tex_id;
}
extern pkid RICO_load_texture_color(u32 pack_id, const char *name,
                                    struct vec4 color)
{
    u8 rgba[4] = {
        (u8)(color.r * 255),
        (u8)(color.g * 255),
        (u8)(color.b * 255),
        (u8)(color.a * 255)
    };
    return RICO_load_texture(pack_id, name, GL_TEXTURE_2D, 1, 1, 32, rgba);
}
extern pkid RICO_load_material(u32 pack_id, const char *name, pkid tex_albedo,
                               pkid tex_mrao, pkid tex_emission)
{
    struct pack *pack = packs[pack_id];
    struct RICO_material *mat = blob_start(pack, RICO_HND_MATERIAL, 0, name);
    mat->tex_albedo = tex_albedo;
    mat->tex_mrao = tex_mrao;
    mat->tex_emission = tex_emission;

    pkid pkid = mat->uid.pkid;
    blob_end(pack);
    return pkid;
}
extern pkid RICO_load_font_file(u32 pack_id, const char *name,
                                const char *filename)
{
    enum RICO_error err;
    pkid font_id = 0;

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
    struct RICO_font *font = blob_start(packs[pack_id], RICO_HND_FONT, 0, name);
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

    font_id = font->uid.pkid;
    blob_end(packs[pack_id]);

    //------------------------------------------------------------
    // Font texture
    //------------------------------------------------------------
    struct RICO_texture *tex0 = blob_start(packs[pack_id], RICO_HND_TEXTURE, 0,
                                           name);
    tex0->width = tex_width;
    tex0->height = tex_height;
    tex0->bpp = tex_bpp;
    tex0->gl_target = GL_TEXTURE_2D;  // Fonts are always 2D textures

    tex0->pixels_offset = blob_offset(packs[pack_id]);
    push_data(packs[pack_id], &buffer[MAP_DATA_OFFSET], tex_width * tex_height,
              tex_bpp / 8);

    pkid tex0_id = tex0->uid.pkid;
    blob_end(packs[pack_id]);

    font = RICO_pack_lookup(font_id);
    font->tex_id = tex0_id;

cleanup:
    free(buffer);
    return font_id;
}
extern pkid RICO_load_mesh(u32 pack_id, const char *name, u32 vertex_size,
                           u32 vertex_count, const void *vertex_data,
                           u32 element_count, const GLuint *element_data)
{
    struct pack *pack = packs[pack_id];
    struct RICO_mesh *mesh = blob_start(pack, RICO_HND_MESH, 0, name);
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
extern pkid RICO_load_string(u32 pack_id, enum RICO_string_slot slot, float x,
                             float y, struct vec4 color, u32 lifespan,
                             pkid font, const char *text)
{
    struct pack *pack = packs[pack_id];
#if RICO_DEBUG_STRING
    printf("[strg][init] name=%s\n", name);
#endif

    const char *name = RICO_string_slot_string[slot];

    // TODO: Reuse mesh and material if they are the same
    // Generate font mesh and get texture handle
    u32 font_mesh_id;
    u32 font_tex_id;
    font_render(&font_mesh_id, &font_tex_id, font, x, y, color, text, name);

    pkid str_object_id =
        RICO_load_object(pack_id, RICO_OBJECT_TYPE_STRING_SCREEN, 0, name);
    struct RICO_object *str_object = RICO_pack_lookup(str_object_id);
    str_object->mesh_id = font_mesh_id;

    struct RICO_string *str = blob_start(pack, RICO_HND_STRING, 0, "bloop");
    str->slot = slot;
    str->object_id = str_object->uid.pkid;
    str->lifespan = lifespan;

    // Store in slot table if not dynamic
    if (str->slot != STR_SLOT_DYNAMIC)
    {
        global_string_slots[str->slot] = str->uid.pkid;
    }

    pkid pkid = str->uid.pkid;
    blob_end(pack);
    return pkid;
}
extern int RICO_load_obj_file(u32 pack_id, const char *filename,
                              pkid *_last_mesh_id)
{
    enum RICO_error err;
    u32 last_mesh_id = 0;

    // TODO: Colossal waste of memory, just preprocess the file and bucket_count them!
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
        tok = dlb_strsep_c(&buffer_ptr, '\n');

        // New object
        if (str_starts_with(tok, "o "))
        {
            if (idx_vertex > 0)
            {
                last_mesh_id =
                    RICO_load_mesh(pack_id, name, sizeof(*vertices), idx_vertex,
                                   vertices, idx_element, elements);
                struct RICO_mesh *mesh = RICO_pack_lookup(last_mesh_id);
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
                vert = dlb_strsep_c(&tok_ptr, ' ');
                vert_pos = dlb_atol(dlb_strsep_c(&vert, '/'));
                vert_tex = dlb_atol(dlb_strsep_c(&vert, '/'));
                vert_norm = dlb_atol(dlb_strsep_c(&vert, '/'));

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
        last_mesh_id =
            RICO_load_mesh(pack_id, name, sizeof(*vertices), idx_vertex,
                           vertices, idx_element, elements);
        struct RICO_mesh *mesh = RICO_pack_lookup(last_mesh_id);
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
