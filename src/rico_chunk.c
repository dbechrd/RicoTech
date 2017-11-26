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
    u32 pool_sizes[POOL_COUNT];
    u32 poolhouse_size = 0;
    for (int i = 0; i < POOL_COUNT; ++i)
    {
        u32 pool_size = POOL_SIZE((*pool_counts)[i], pool_item_sizes[i]);
        pool_sizes[i] = pool_size;
        poolhouse_size += pool_size;
    }
    u32 cereal_size = chunk_size + poolhouse_size;
    u32 transient_size = poolhouse_size;
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
    u8 *ptr = (u8 *)chunk;
    u32 offset = 0;
    offset += chunk_size;
    for (enum rico_persist persist = 0; persist < PERSIST_COUNT; ++persist)
    {
        for (int i = 0; i < POOL_COUNT; ++i)
        {
            chunk->pools[persist][i] = (struct rico_pool *)(ptr + offset);
            offset += pool_sizes[i];
#if 0
            char dbg[32] = { 0 };
            snprintf(dbg, sizeof(dbg), "%d %d %d\n", persist, i, (int)offset - (int)chunk);
            OutputDebugStringA(dbg);
#endif
            char pool_name[32] = { 0 };
            snprintf(pool_name, sizeof(pool_name), "%s_%s",
                     rico_pool_itemtype_string[i], rico_persist_string[persist]);

            pool_init(chunk->pools[persist][i], persist, pool_name,
                      chunk->pool_counts[i], pool_item_sizes[i],
                      pool_item_fixed_counts[i]);
        }
    }

    RICO_ASSERT(offset == chunk->total_size);

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
    u8 *seek = (u8 *)chunk + skip;
    u32 bytes = chunk->cereal_size - skip;

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
    fread(&tmp_chunk->total_size,  sizeof(tmp_chunk->total_size),  1, file->fs);
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
    u8 *ptr = (u8 *)chunk;
    u32 skip = sizeof(chunk->uid) + sizeof(chunk->total_size) +
               sizeof(chunk->cereal_size);
    u8 *seek = ptr + skip;
    u32 bytes = chunk->cereal_size - skip;

    // TODO: Check fread success
    fread(seek, bytes, 1, file->fs);

    // Calculate pool sizes
    u32 pool_sizes[POOL_COUNT];
    for (int i = 0; i < POOL_COUNT; ++i)
    {
        u32 pool_size = POOL_SIZE(chunk->pool_counts[i], pool_item_sizes[i]);
        pool_sizes[i] = pool_size;
    }

    // Fix pool pointers
    u32 offset = 0;
    offset += sizeof(struct rico_chunk);
    for (enum rico_persist persist = 0; persist < PERSIST_COUNT; ++persist)
    {
        for (int i = 0; i < POOL_COUNT; ++i)
        {
            chunk->pools[persist][i] = (struct rico_pool *)(ptr + offset);
            offset += pool_sizes[i];
#if 0
            char dbg[32] = { 0 };
            snprintf(dbg, sizeof(dbg), "%d %d %d\n", persist, i, (int)offset - (int)chunk);
            OutputDebugStringA(dbg);
#endif
            if (persist == PERSISTENT)
            {
                // Fix member pointers
                pool_fixup(chunk->pools[persist][i]);
            }
            else if (persist == TRANSIENT)
            {
                // Reinitialize transient pools, which aren't stored in save files
                char pool_name[32];
                snprintf(pool_name, sizeof(pool_name), "%s_%s",
                         rico_pool_itemtype_string[i],
                         rico_persist_string[persist]);

                pool_init(chunk->pools[persist][i], persist, pool_name,
                          chunk->pool_counts[i], pool_item_sizes[i],
                          pool_item_fixed_counts[i]);
            }
        }
    }

    RICO_ASSERT(offset == chunk->total_size);

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

    for (int i = 0; i < POOL_COUNT; ++i)
    {
        for (int type = 0; type < PERSIST_COUNT; ++type)
        {
            printf("             %s = %d\n",
                   chunk->pools[type][i]->uid.name,
                   chunk->pool_counts[i]);
        }
    }
}
#endif
