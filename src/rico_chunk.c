#define ADD_BYTES(ptr, offset) ((u8 *)ptr += offset)
#define PTR_SUBTRACT(a, b) ((u8 *)a - (u8 *)b)

// TODO: Is a global chunk the best way to keep track of the pools?
global struct rico_chunk *global_chunk;

internal void chunk_print(struct rico_chunk *chunk);

int chunk_init(const char *name, u32 strings, u32 fonts, u32 textures,
               u32 materials, u32 meshes, u32 objects,
               struct rico_chunk **_chunk)
{
#if RICO_DEBUG_CHUNK
    printf("[chnk][init] name=%s\n", name);
#endif

    u32 chunkSize = sizeof(struct rico_chunk);
    u32 pool1 = POOL_SIZE(strings,   RICO_STRING_SIZE);
    u32 pool2 = POOL_SIZE(fonts,     RICO_FONT_SIZE);
    u32 pool3 = POOL_SIZE(textures,  RICO_TEXTURE_SIZE);
    u32 pool4 = POOL_SIZE(materials, RICO_MATERIAL_SIZE);
    u32 pool5 = POOL_SIZE(meshes,    RICO_MESH_SIZE);
    u32 pool6 = POOL_SIZE(objects,   RICO_OBJECT_SIZE);
    u32 total_size = chunkSize + pool1 + pool2 + pool3 + pool4 + pool5 + pool6;

    // TODO: Memory management
    void *mem_block = calloc(1, total_size);
    if (!mem_block) return RICO_ERROR(ERR_BAD_ALLOC,
                                      "Failed to alloc memory for chunk %s",
                                      name);

    //memset(mem_block, INT_MAX, total_size);

    struct rico_chunk *chunk = (struct rico_chunk *)mem_block;
    uid_init(&chunk->uid, RICO_UID_CHUNK, name, true);
    chunk->total_size      = total_size;
    chunk->count_strings   = strings;
    chunk->count_fonts     = fonts;
    chunk->count_textures  = textures;
    chunk->count_materials = materials;
    chunk->count_meshes    = meshes;
    chunk->count_objects   = objects;

    u8 *offset = (u8 *)chunk; ADD_BYTES(offset, chunkSize);
    chunk->strings   = (struct rico_pool *)offset; ADD_BYTES(offset, pool1);
    chunk->fonts     = (struct rico_pool *)offset; ADD_BYTES(offset, pool2);
    chunk->textures  = (struct rico_pool *)offset; ADD_BYTES(offset, pool3);
    chunk->materials = (struct rico_pool *)offset; ADD_BYTES(offset, pool4);
    chunk->meshes    = (struct rico_pool *)offset; ADD_BYTES(offset, pool5);
    chunk->objects   = (struct rico_pool *)offset; ADD_BYTES(offset, pool6);
    RICO_ASSERT(PTR_SUBTRACT(offset, chunk) == chunk->total_size);

    pool_init(chunk->strings,   "Strings",   strings,   RICO_STRING_SIZE,   STR_SLOT_DYNAMIC);
    pool_init(chunk->fonts,     "Fonts",     fonts,     RICO_FONT_SIZE,     0);
    pool_init(chunk->textures,  "Textures",  textures,  RICO_TEXTURE_SIZE,  0);
    pool_init(chunk->materials, "Materials", materials, RICO_MATERIAL_SIZE, 0);
    pool_init(chunk->meshes,    "Meshes",    meshes,    RICO_MESH_SIZE,     0);
    pool_init(chunk->objects,   "Objects",   objects,   RICO_OBJECT_SIZE,   0);

#if RICO_DEBUG_CHUNK
    chunk_print(chunk);
#endif

    *_chunk = chunk;
    return SUCCESS;
}

void chunk_active_set(struct rico_chunk *chunk)
{
    global_chunk = chunk;
}

struct rico_chunk *chunk_active()
{
    return global_chunk;
}

//int chunk_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(chunk_serialize_0)
{
    enum rico_error err = SUCCESS;
    const struct rico_chunk *chunk = handle;

    // Write chunk to file
    u32 skip = sizeof(chunk->uid);
    u32 bytes = chunk->total_size - skip;
    u8 *seek = (u8 *)chunk; ADD_BYTES(seek, skip);

    // TODO: Don't write entire pools. Write pool header (w/ size) and then only
    //       the handles in use. No point writing empty pool slots to disk. How
    //       different is this from doing compression in RAM before writing to
    //       disk?

    // TODO: Check fwrite success
    fwrite(seek, bytes, 1, file->fs);

    #if RICO_DEBUG_CHUNK
        printf("[chnk][save] uid=%d name=%s filename=%s total_size=%d\n",
               chunk->uid.uid, chunk->uid.name, file->filename,
               chunk->total_size);
    #endif

    return err;
}

//int chunk_deserialize_0(void *_handle, const struct rico_file *file)
DESERIAL(chunk_deserialize_0)
{
    enum rico_error err = SUCCESS;
    struct rico_chunk *tmp_chunk = *_handle;

#if RICO_DEBUG_CHUNK
    printf("[chnk][load] filename=%s\n", file->filename);
#endif

    // Read chunk size from file so we know how much to allocate, then
    // initialize the chunk properly before calling fread().
    fread(&tmp_chunk->total_size, sizeof(tmp_chunk->total_size), 1, file->fs);

    // TODO: Memory management
    void *mem_block = calloc(1, tmp_chunk->total_size);
    if (!mem_block) return RICO_ERROR(ERR_BAD_ALLOC,
                                      "Failed to alloc memory for chunk %s",
                                      tmp_chunk->uid.name);

    struct rico_chunk *chunk = mem_block;
    memcpy(chunk, tmp_chunk, sizeof(struct rico_chunk));

    // TODO: Set pool pointers first, then read pools from file and skip the
    //       unused storage space (which is pointless to store in the file).

    // Read chunk from file
    u32 skip = sizeof(chunk->uid) + sizeof(chunk->total_size);
    u32 bytes = chunk->total_size - skip;
    u8 *seek = (u8 *)chunk; ADD_BYTES(seek, skip);

    // TODO: Check fread success
    fread(seek, bytes, 1, file->fs);

    // Fix pool pointers
    u32 chunkSize = sizeof(struct rico_chunk);
    u32 pool1 = POOL_SIZE(chunk->count_strings,   RICO_STRING_SIZE);
    u32 pool2 = POOL_SIZE(chunk->count_fonts,     RICO_FONT_SIZE);
    u32 pool3 = POOL_SIZE(chunk->count_textures,  RICO_TEXTURE_SIZE);
    u32 pool4 = POOL_SIZE(chunk->count_materials, RICO_MATERIAL_SIZE);
    u32 pool5 = POOL_SIZE(chunk->count_meshes,    RICO_MESH_SIZE);
    u32 pool6 = POOL_SIZE(chunk->count_objects,   RICO_OBJECT_SIZE);

    u8 *offset = (u8 *)chunk; ADD_BYTES(offset, chunkSize);
    chunk->strings   = (struct rico_pool *)offset; ADD_BYTES(offset, pool1);
    chunk->fonts     = (struct rico_pool *)offset; ADD_BYTES(offset, pool2);
    chunk->textures  = (struct rico_pool *)offset; ADD_BYTES(offset, pool3);
    chunk->materials = (struct rico_pool *)offset; ADD_BYTES(offset, pool4);
    chunk->meshes    = (struct rico_pool *)offset; ADD_BYTES(offset, pool5);
    chunk->objects   = (struct rico_pool *)offset; ADD_BYTES(offset, pool6);
    RICO_ASSERT(PTR_SUBTRACT(offset, chunk) == chunk->total_size);

    pool_fixup(chunk->strings);
    pool_fixup(chunk->fonts);
    pool_fixup(chunk->textures);
    pool_fixup(chunk->materials);
    pool_fixup(chunk->meshes);
    pool_fixup(chunk->objects);

#if RICO_DEBUG_CHUNK
    chunk_print(chunk);
#endif

    *_handle = chunk;
    return err;
}

#if RICO_DEBUG_CHUNK
internal void chunk_print(struct rico_chunk *chunk)
{
    // Print information about chunk and pool sizes
    printf("[chnk][show] uid=%d name=%s total_size=%d\n" \
           "     Strings = %d\n" \
           "       Fonts = %d\n" \
           "    Textures = %d\n" \
           "   Materials = %d\n" \
           "      Meshes = %d\n" \
           "     Objects = %d\n",
           chunk->uid.uid, chunk->uid.name, chunk->total_size,
           chunk->count_strings,
           chunk->count_fonts,
           chunk->count_textures,
           chunk->count_materials,
           chunk->count_meshes,
           chunk->count_objects);
}
#endif