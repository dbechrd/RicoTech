// TODO: Is a global chunk the best way to keep track of the pools?
//       No.. duh. Get rid of this! Or at very least make it an array/pool.
struct rico_chunk *chunk_active;
struct rico_chunk *chunk_transient;

internal void chunk_print(struct rico_chunk *chunk);

int chunk_init(struct rico_chunk **_chunk, const char *name,
               const chunk_pool_counts *pool_counts)
{
#if RICO_DEBUG_CHUNK
    printf("[chnk][init] name=%s\n", name);
#endif

    // Calculate pool sizes
    u32 pool_sizes[RICO_HND_CEREAL_COUNT];
    u32 poolhouse_size = 0;
    for (int i = 0; i < RICO_HND_CEREAL_COUNT; ++i)
    {
        u32 pool_size = POOL_SIZE((*pool_counts)[i], size_by_handle[i]);
        pool_sizes[i] = pool_size;
        poolhouse_size += pool_size;
    }
    u32 total_size = sizeof(struct rico_chunk) + poolhouse_size;

    // TODO: Memory management
    void *mem_block = calloc(1, total_size);
    if (!mem_block)
    {
        return RICO_ERROR(ERR_BAD_ALLOC, "Failed to alloc memory for chunk %s",
                          name);
    }

    // Initialize chunk
    struct rico_chunk *chunk = (struct rico_chunk *)mem_block;
    hnd_init(&chunk->hnd, RICO_HND_CHUNK, name);
    chunk->total_size = total_size;
    memcpy(&chunk->pool_counts, pool_counts, sizeof(chunk->pool_counts));

    // Initialize pools
    u8 *ptr = (u8 *)chunk;
    u32 offset = 0;
    offset += sizeof(struct rico_chunk);
    for (int i = 0; i < RICO_HND_CEREAL_COUNT; ++i)
    {
        chunk->pools[i] = (struct rico_pool *)(ptr + offset);
        offset += pool_sizes[i];
#if 0
        char dbg[32] = { 0 };
        snprintf(dbg, sizeof(dbg), "%d %d\n", i, (int)offset - (int)chunk);
        OutputDebugStringA(dbg);
#endif
        char pool_name[32] = { 0 };
        snprintf(pool_name, sizeof(pool_name), "%s",
                 rico_hnd_type_string[i]);

        pool_init(chunk->pools[i], pool_name, chunk->pool_counts[i],
                  size_by_handle[i]);
    }

    RICO_ASSERT(offset == chunk->total_size);

#if RICO_DEBUG_CHUNK
    chunk_print(chunk);
#endif

    *_chunk = chunk;
    return SUCCESS;
}

int chunk_alloc(struct rico_chunk *chunk, enum rico_hnd_type type,
                struct hnd **_handle)
{
    RICO_ASSERT(type < RICO_HND_CEREAL_COUNT);
    enum rico_error err;

    struct rico_pool *pool = chunk->pools[type];
    err = pool_handle_alloc(pool, _handle);
    if (err) return err;

    (*_handle)->chunk = chunk;
    return err;
}

int chunk_free(struct rico_chunk *chunk, struct hnd *handle)
{
    RICO_ASSERT(handle);

    hashtable_delete_uid(&global_uids, handle->uid);

    handle->chunk = NULL;
    struct rico_pool *pool = chunk->pools[handle->type];
    return pool_handle_free(pool, handle);
}

int chunk_serialize(const struct rico_chunk *chunk,
                    const struct rico_file *file)
{
    enum rico_error err = SUCCESS;

    // Write chunk to file
    u8 *seek = (u8 *)chunk;
    u32 bytes = chunk->total_size;

    // TODO: Don't write entire pools. Write pool header (w/ size) and then only
    //       the handles in use. No point writing empty pool slots to disk. How
    //       different is this from doing compression in RAM before writing to
    //       disk?

    // TODO: Check fwrite success
    fwrite(seek, bytes, 1, file->fs);

    #if RICO_DEBUG_CHUNK
        printf("[chnk][save] uid=%d name=%s filename=%s total_size=%d\n",
               chunk->hnd.uid, chunk->hnd.name, file->filename,
               chunk->total_size);
    #endif

    return err;
}

int chunk_deserialize(struct rico_chunk **_chunk, const struct rico_file *file)
{
    enum rico_error err = SUCCESS;

#if RICO_DEBUG_CHUNK
    printf("[chnk][load] filename=%s\n", file->filename);
#endif

    // Read chunk size from file so we know how much to allocate, then
    // initialize the chunk properly before calling fread().
    struct rico_chunk tmp_chunk = { 0 };
    fread(&tmp_chunk, sizeof(struct rico_chunk), 1, file->fs);

    // TODO: Memory management
    void *mem_block = calloc(1, tmp_chunk.total_size);
    if (!mem_block)
    {
        return RICO_ERROR(ERR_BAD_ALLOC, "Failed to alloc memory for chunk %s",
                          tmp_chunk.hnd.name);
    }

    struct rico_chunk *chunk = mem_block;
    memcpy(chunk, &tmp_chunk, sizeof(struct rico_chunk));

    // Read chunk from file
    u8 *ptr = (u8 *)chunk;
    u32 skip = sizeof(struct rico_chunk);
    u8 *seek = ptr + skip;
    u32 bytes = chunk->total_size - skip;

    // TODO: Check fread success
    fread(seek, bytes, 1, file->fs);

    // Calculate pool sizes
    u32 pool_sizes[RICO_HND_CEREAL_COUNT];
    for (int i = 0; i < RICO_HND_CEREAL_COUNT; ++i)
    {
        u32 pool_size = POOL_SIZE(chunk->pool_counts[i], size_by_handle[i]);
        pool_sizes[i] = pool_size;
    }

    // Fix pool pointers
    u32 offset = 0;
    offset += sizeof(struct rico_chunk);
    for (int i = 0; i < RICO_HND_CEREAL_COUNT; ++i)
    {
        chunk->pools[i] = (struct rico_pool *)(ptr + offset);
        offset += pool_sizes[i];
#if 0
        char dbg[32] = { 0 };
        snprintf(dbg, sizeof(dbg), "%d %d\n", i, (int)offset - (int)chunk);
        OutputDebugStringA(dbg);
#endif
        // Fix pool pointers (and collect UIDs)
        pool_fixup(chunk->pools[i]);
    }

    RICO_ASSERT(offset == chunk->total_size);

    for (int i = 0; i < RICO_HND_CEREAL_COUNT; ++i)
    {
        // Fix handle pointers
        pool_fixup_handles(chunk->pools[i], chunk, i);
    }

#if RICO_DEBUG_CHUNK
    chunk_print(chunk);
#endif

    *_chunk = chunk;
    return err;
}

#if RICO_DEBUG_CHUNK
internal void chunk_print(struct rico_chunk *chunk)
{
    // Print information about chunk and pool sizes
    printf("[chnk][show] uid=%d name=%s total_size=%d\n",
           chunk->hnd.uid, chunk->hnd.name, chunk->total_size);

    for (int i = 0; i < RICO_HND_CEREAL_COUNT; ++i)
    {
        printf("             %s = %d\n", chunk->pools[i]->hnd.name,
               chunk->pool_counts[i]);
    }
}
#endif
