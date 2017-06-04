#define ADD_BYTES(ptr, offset) ((u8 *)ptr += offset)
#define PTR_SUBTRACT(a, b) ((u8 *)a - (u8 *)b)

// TODO: Is a global chunk the best way to keep track of the pools?
global struct rico_chunk *global_chunk;

internal void chunk_print(struct rico_chunk *chunk);

int chunk_init(struct rico_chunk **_chunk, const char *name,
               const chunk_pool_counts *pool_counts)
{
#if RICO_DEBUG_CHUNK
    printf("[chnk][init] name=%s\n", name);
#endif

    u32 chunk_size = sizeof(struct rico_chunk);
    
    // Calculate pool sizes
    u32 pool_sizes[POOL_ITEMTYPE_COUNT];
    u32 cereal_size = 0;
    for (int i = 0; i < POOL_ITEMTYPE_COUNT; ++i)
    {
        u32 pool_size = POOL_SIZE((*pool_counts)[i], pool_item_sizes[i]);
        pool_sizes[i] = pool_size;
        cereal_size += pool_size;
    }
    u32 transient_size = cereal_size;
    u32 total_size = cereal_size + transient_size;

    // TODO: Memory management
    void *mem_block = calloc(1, total_size);
    if (!mem_block)
    {
        return RICO_ERROR(ERR_BAD_ALLOC, "Failed to alloc memory for chunk %s",
                          name);
    }

    // Initialize chunk
    struct rico_chunk *chunk = (struct rico_chunk *)mem_block;
    uid_init(&chunk->uid, RICO_UID_CHUNK, name, true);
    chunk->total_size = total_size;
    chunk->cereal_size = cereal_size;
    memcpy(&chunk->pool_counts, pool_counts, sizeof(chunk->pool_counts));

    // Initialize pools
    u8 *offset = (u8 *)chunk;
    ADD_BYTES(offset, chunk_size);
    for (int i = 0; i < POOL_ITEMTYPE_COUNT; ++i)
    {
        for (int type = 0; type < PERSIST_COUNT; ++type)
        {
            chunk->pools[i][type] = (struct rico_pool *)offset;
            ADD_BYTES(offset, pool_sizes[i]);

            char pool_name[32];
            snprintf("%s_%s", sizeof(pool_name), rico_pool_item_type_string[i],
                     rico_persist_string[i]);

            pool_init(chunk->pools[i], pool_name, chunk->pool_counts[i],
                      pool_item_sizes[i], pool_item_fixed_counts[i]);
        }
    }
    RICO_ASSERT(PTR_SUBTRACT(offset, chunk) == chunk->total_size);

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
    u32 bytes = chunk->cereal_size - skip;
    u8 *seek = (u8 *)chunk; ADD_BYTES(seek, skip);

    // TODO: Don't write entire pools. Write pool header (w/ size) and then only
    //       the handles in use. No point writing empty pool slots to disk. How
    //       different is this from doing compression in RAM before writing to
    //       disk?

    // TODO: Check fwrite success
    fwrite(seek, bytes, 1, file->fs);

    #if RICO_DEBUG_CHUNK
        printf("[chnk][save] uid=%d name=%s filename=%s total_size=%d cereal_size=%d\n",
               chunk->uid.uid, chunk->uid.name, file->filename,
               chunk->total_size, chunk->cereal_size);
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
    fread(&tmp_chunk->cereal_size, sizeof(tmp_chunk->cereal_size), 1, file->fs);

    // TODO: Memory management
    void *mem_block = calloc(1, tmp_chunk->total_size);
    if (!mem_block)
    {
        return RICO_ERROR(ERR_BAD_ALLOC, "Failed to alloc memory for chunk %s",
                          tmp_chunk->uid.name);
    }

    struct rico_chunk *chunk = mem_block;
    memcpy(chunk, tmp_chunk, sizeof(struct rico_chunk));

    // Read chunk from file
    u32 skip = sizeof(chunk->uid) + sizeof(chunk->total_size) +
               sizeof(chunk->cereal_size);
    u32 bytes = chunk->cereal_size - skip;
    u8 *seek = (u8 *)chunk; ADD_BYTES(seek, skip);

    // TODO: Check fread success
    fread(seek, bytes, 1, file->fs);

    // Calculate pool sizes
    u32 chunk_size = sizeof(struct rico_chunk);
    u32 pool_sizes[POOL_ITEMTYPE_COUNT];
    for (int i = 0; i < POOL_ITEMTYPE_COUNT; ++i)
    {
        u32 pool_size = POOL_SIZE(chunk->pool_counts[i], pool_item_sizes[i]);
        pool_sizes[i] = pool_size;
    }

    // Fix pool pointers
    u8 *offset = (u8 *)chunk;
    ADD_BYTES(offset, chunk_size);
    for (int i = 0; i < POOL_ITEMTYPE_COUNT; ++i)
    {
        for (int type = 0; type < PERSIST_COUNT; ++type)
        {
            chunk->pools[i][type] = (struct rico_pool *)offset;
            ADD_BYTES(offset, pool_sizes[i]);

            if (type == RICO_PERSISTENT)
            {
                // Fix member pointers
                pool_fixup(chunk->pools[i][type]);
            }
            else if (type == RICO_TRANSIENT)
            {
                // Reinitialize transient pools, which aren't stored in save files
                char pool_name[32];
                snprintf("%s_%s", sizeof(pool_name),
                         rico_pool_item_type_string[i],
                         rico_persist_string[i]);

                pool_init(chunk->pools[i], pool_name,
                          chunk->pool_counts[i], pool_item_sizes[i],
                          pool_item_fixed_counts[i]);
            }
        }
    }
    RICO_ASSERT(PTR_SUBTRACT(offset, chunk) == chunk->total_size);

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
    printf("[chnk][show] uid=%d name=%s total_size=%d cereal_size=%d\n",
           chunk->uid.uid, chunk->uid.name, chunk->total_size,
           chunk->cereal_size);

    for (int i = 0; i < POOL_ITEMTYPE_COUNT; ++i)
    {
        for (int type = 0; type < PERSIST_COUNT; ++type)
        {
            printf("             %s = %d\n",
                   chunk->pools[i][type]->uid.name,
                   chunk->pool_counts[i]);
        }
    }
}
#endif