internal void pool_print_handles(struct rico_pool *pool);

static inline void pool_fixup(struct rico_pool *pool)
{
    // TODO: Could clean this up with PTR_ADD_BYTE macro
    pool->handles = (struct pool_hnd *)((u8 *)pool + POOL_OFFSET_HANDLES());
    pool->buffer = (u8 *)pool + POOL_OFFSET_DATA(pool->block_count);
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
    pool->free = 0;

    // Pointer fix-up
    pool_fixup(pool);

    // Initialize handles
    for (u32 i = 0; i < pool->block_count - 1; ++i)
    {
        pool->handles[i].next = i + 1;
    }

#if RICO_DEBUG_POOL
    printf("[pool][init] %s\n", pool->name);
#endif

    return SUCCESS;
}

int pool_add(struct rico_pool *pool, struct pool_id *_id)
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

    // Take first free handle
    _id->idx = pool->free;
    _id->generation = pool->handles[_id->idx].generation;

    pool->free = pool->handles[pool->free].next;
    pool->blocks_used++;

#if RICO_DEBUG_POOL
    printf("[pool][aloc] %s\n", pool->name);
    pool_print_handles(pool);
#endif

    return err;
}

int pool_remove(struct rico_pool *pool, struct pool_id *id)
{
    RICO_ASSERT(pool);

    if (pool->handles[id->idx].generation != id->generation)
    {
        return RICO_ERROR(ERR_POOL_INVALID_HANDLE,
                          "Generation mismatch, index %u already freed!",
                          id->idx);
    }

    u32 handle = pool->handles[id->idx].handle;
    if (handle < pool->blocks_used - 1)
    {
        // Overwrite deleted block with last block in buffer to keep the block
        // buffer compacted.
        memcpy((void *)pool->buffer[handle * pool->block_size],
               (void *)pool->buffer[(pool->blocks_used - 1) * pool->block_size],
               pool->block_size);

#if RICO_DEBUG
        // Hopefully make out-of-bounds errors more obvious while debugging
        memset((void *)pool->buffer[(pool->blocks_used - 1) * pool->block_size],
               0, pool->block_size);
#endif
    }

    pool->handles[id->idx].generation++;
    pool->handles[id->idx].next = pool->free;
    pool->free = id->idx;
    pool->blocks_used--;

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
    u32 free = pool->free;
    for (u32 i = 0; i < pool->block_count; i++)
    {
        if (i == free)
        {
            free = pool->handles[free].next;
            continue;
        }

        printf("%u", pool->handles[i].handle);
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

#if 0
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