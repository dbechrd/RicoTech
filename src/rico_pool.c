internal void pool_print_handles(struct rico_pool *pool);

inline void pool_fixup(struct rico_pool *pool)
{
    // TODO: Could clean this up with PTR_ADD_BYTE macro
    pool->tags = (struct block_tag *)((u8 *)pool + POOL_OFFSET_HANDLES());
    pool->blocks = (u8 *)pool + POOL_OFFSET_DATA(pool->block_count);
    pool->end = pool->blocks + (pool->blocks_used * pool->block_size);
    
    struct hnd *hnd;
    for (u32 i = 0; i < pool->block_count - 1; ++i)
    {
        hnd = (struct hnd *)(pool->blocks + (i * pool->block_size));
        hnd->pool = pool;
    }
}

// TODO: Allocate from heap pool, don't keep calling calloc
int pool_init(void *buf, const char *name, u32 block_count, u32 block_size)
{
    RICO_ASSERT(block_count > 0);
    RICO_ASSERT(block_size > 0);

    struct rico_pool *pool = buf;
    strncpy(pool->name, name, sizeof(pool->name) - 1);
    pool->block_count = block_count;
    pool->block_size = block_size;
    pool->blocks_used = 0;
    pool->next_free = 0;

    // Pointer fix-up
    pool_fixup(pool);

    // Initialize handles
    for (u32 i = 0; i < pool->block_count - 1; ++i)
    {
        pool->tags[i].next_free = i + 1;
    }

#if RICO_DEBUG_POOL
    printf("[pool][init] %s\n", pool->name);
#endif

    return SUCCESS;
}

int pool_add(struct rico_pool *pool, struct pool_id *id)
{
    RICO_ASSERT(pool);

    enum rico_error err = SUCCESS;

    if (pool->blocks_used == pool->block_count)
    {
#if RICO_DEBUG_POOL
        printf("[pool][ ERR] %s out of memory. [%d] blocks used.\n",
               pool->name, pool->blocks_used);
        pool_print_handles(pool);
#endif

        return RICO_FATAL(ERR_POOL_OUT_OF_MEMORY, "%s pool is full",
                          pool->name);
    }

    id->tag = pool->next_free;
    pool->next_free = pool->tags[id->tag].next_free;

    id->generation = pool->tags[id->tag].generation;
    pool->tags[id->tag].ref_count = 1;
    pool->tags[id->tag].block_idx = pool->blocks_used;
    pool->tags[pool->tags[id->tag].block_idx].tag_idx = id->tag;
    pool->blocks_used++;
    pool->end = pool->blocks + (pool->blocks_used * pool->block_size);

#if RICO_DEBUG_POOL
    printf("[pool][aloc] %s\n", pool->name);
    pool_print_handles(pool);
#endif

    return err;
}

int pool_remove(struct rico_pool *pool, struct pool_id id)
{
    RICO_ASSERT(pool);

    if (pool->tags[id.tag].generation != id.generation)
    {
        return RICO_ERROR(ERR_POOL_BAD_FREE,
                          "Tag generation mismatch, tag %u already freed!",
                          id.tag);
    }
    pool->tags[id.tag].generation++;

    if (pool->tags[id.tag].ref_count == 0)
    {
        return RICO_ERROR(ERR_POOL_BAD_FREE,
                          "Tag ref_count is zero, tag %u already freed!",
                          id.tag);
    }
    pool->tags[id.tag].ref_count--;
    if (pool->tags[id.tag].ref_count > 0)
        return SUCCESS;

    u32 block_idx = pool->tags[id.tag].block_idx;
    RICO_ASSERT(block_idx < pool->blocks_used);
    pool->blocks_used--;
    pool->end = pool->blocks + (pool->blocks_used * pool->block_size);

    if (block_idx < pool->blocks_used)
    {
        // Fill hole with last block to keep buffer contiguous
        memcpy(pool->blocks + (block_idx * pool->block_size),
               pool->blocks + (pool->blocks_used * pool->block_size),
               pool->block_size);

        // Update tag index for the block that we moved
        pool->tags[block_idx].tag_idx = pool->tags[pool->blocks_used].tag_idx;

#if RICO_DEBUG
        // Zero memory block to force out-of-bounds errors while debugging
        memset(pool->blocks + (pool->blocks_used * pool->block_size),
               0, pool->block_size);
#endif
    }

    pool->tags[id.tag].next_free = pool->next_free;
    pool->next_free = id.tag;

#if RICO_DEBUG_POOL
    printf("[pool][free] %s\n",
           pool->name);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}

internal void pool_print_handles(struct rico_pool *pool)
{
#if RICO_DEBUG_POOL
    printf("             [");
    if (pool->blocks_used == 0)
    {
        printf("EMPTY]\n");
        return;
    }

    // Print pool
    u32 count = 0;
    u32 free = pool->next_free;
    for (u32 i = 0; i < pool->block_count; i++)
    {
        if (i == free)
        {
            free = pool->tags[free].next_free;
            continue;
        }

        printf("%u", pool->tags[i].block_idx);
        count++;
        if (count == pool->blocks_used) break;
        printf(" ");
    }
    printf("]\n");
    fflush(stdout);
#else
    UNUSED(pool);
#endif
}

inline void *pool_first(struct rico_pool *pool)
{
    if (pool->blocks_used == 0) return NULL;
    return pool->blocks;
}

inline void *pool_last(struct rico_pool *pool)
{
    if (pool->blocks_used == 0) return NULL;
    return pool->end - pool->block_size;
}

inline void *pool_next(struct rico_pool *pool, void *block)
{
    if (block == NULL) return pool_first(pool);
    block = (u8 *)block + pool->block_size;
    return (block == pool->end) ? NULL : block;
}

inline void *pool_prev(struct rico_pool *pool, void *block)
{
    if (block == NULL) return pool_last(pool);
    return (block == pool->blocks) ? NULL : (u8 *)block - pool->block_size;
}

inline void *pool_read(struct rico_pool *pool, struct pool_id id)
{
    return pool->blocks + (pool->tags[id.tag].block_idx * pool->block_size);
}

#if 0
struct hnd *pool_handle_next(struct rico_pool *pool, u32 idx)
{
    RICO_ASSERT(pool);

    // Nothing selected, return first item in pool
    if (!handle)
        return pool_handle_first(pool);

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->blocks_used) {}

    // Last item selected, return first item in pool
    if (i == pool->blocks_used)
        return pool_handle_first(pool);

    // Return next item
    return pool->handles[i];
}

struct hnd *pool_handle_prev(struct rico_pool *pool, u32 idx)
{
    RICO_ASSERT(pool);

    // Nothing selected, return last item in pool
    if (!handle)
        return pool_handle_last(pool);

    // First item selected, return last item in pool
    if (handle->uid == pool->handles[0]->uid)
        return pool->handles[pool->blocks_used - 1];

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->blocks_used) {}

    // Return previous item
    return pool->handles[i - 1];
}

struct hnd *pool_handle_first(struct rico_pool *pool)
{
    // Pool is empty, return null
    if (pool->blocks_used == 0)
        return NULL;

    return pool->handles[0];
}

struct hnd *pool_handle_last(struct rico_pool *pool)
{
    // Pool is empty, return null
    if (pool->blocks_used == 0)
        return NULL;

    return pool->handles[pool->blocks_used - 1];
}

struct hnd *pool_handle_next(struct rico_pool *pool, u32 idx)
{
    RICO_ASSERT(pool);

    // Nothing selected, return first item in pool
    if (!handle)
        return pool_handle_first(pool);

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->blocks_used) {}

    // Last item selected, return first item in pool
    if (i == pool->blocks_used)
        return pool_handle_first(pool);

    // Return next item
    return pool->handles[i];
}

struct hnd *pool_handle_prev(struct rico_pool *pool, u32 idx)
{
    RICO_ASSERT(pool);

    // Nothing selected, return last item in pool
    if (!handle)
        return pool_handle_last(pool);

    // First item selected, return last item in pool
    if (handle->uid == pool->handles[0]->uid)
        return pool->handles[pool->blocks_used - 1];

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->blocks_used) {}

    // Return previous item
    return pool->handles[i - 1];
}

//int pool_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(pool_serialize_0)
{
    enum rico_error err;

    const struct rico_pool *pool = handle;
    fwrite(&pool->count,        sizeof(pool->count),        1, file->fs);
    fwrite(&pool->size,         sizeof(pool->size),         1, file->fs);
    fwrite(&pool->active,       sizeof(pool->active),       1, file->fs);
    fwrite(pool->handles, sizeof(*pool->handles), pool->count, file->fs);
    for (u32 i = 0; i < pool->active; ++i)
    {
        err = rico_serialize(pool_read(pool, pool->handles[i]), file);
        if (err == ERR_SERIALIZE_DISABLED)
            continue;
        if (err)
            return err;
    }

    return SUCCESS;
}

//int pool_deserialize_0(void *_handle, const struct rico_file *file)
DESERIAL(pool_deserialize_0)
{
    enum rico_error err;

    struct rico_pool *pool = *_handle;
    fread(&pool->count,        sizeof(pool->count),        1, file->fs);
    fread(&pool->size,         sizeof(pool->size),         1, file->fs);
    fread(&pool->active,       sizeof(pool->active),       1, file->fs);

    if(pool->count > 0)
    {
        pool->handles = calloc(pool->count, sizeof(u32));
        if (!pool->handles)
            return RICO_ERROR(ERR_BAD_ALLOC,
                              "Failed to alloc handles for pool %s",
                              pool->name);

        pool->data = calloc(pool->count, pool->size);
        if (!pool->data)
            return RICO_ERROR(ERR_BAD_ALLOC, "Failed to alloc data for pool %s",
                              pool->name);

        fread(pool->handles, sizeof(*pool->handles), pool->count, file->fs);
        for (u32 i = 0; i < pool->active; ++i)
        {
            err = rico_deserialize(pool_read(pool, pool->handles[i]), file);
            if (err == ERR_SERIALIZE_DISABLED)
                continue;
            if (err)
                return err;
        }

        if (pool->active > 0)
        {
            struct rico_uid *uid;
            struct handle handle;
            for (u32 i = 0; i < pool->active; ++i)
            {
                uid = pool_read(pool, pool->handles[i]);
                if (uid->uid == UID_NULL)
                {
                    pool->active--;
                    if (pool->active > 0)
                    {
                        handle = pool->handles[i];
                        pool->handles[i] = pool->handles[pool->active];
                        pool->handles[pool->active] = handle;
                    }
                }
            }
        }
    }

#if RICO_DEBUG_POOL
    printf("[pool][load] %s file %s\n",
           pool->name, file->filename);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}
#endif