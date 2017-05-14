static void pool_print_handles(struct rico_pool *pool);

// TODO: Allocate from heap pool, don't keep calling calloc
int pool_init(void *mem_block, const char *name, u32 count, u32 size,
              u32 static_count)
{
    RICO_ASSERT(count > 0);

    struct rico_pool *pool = mem_block;
    uid_init(&pool->uid, RICO_UID_POOL, name, true);
    pool->count = count;
    pool->size = size;
    pool->static_count = static_count;
    pool->active = static_count;

    // TODO: Do these work..?
    pool_fixup(pool);

    // TODO: Should I use UIDs for handles instead of e.g. 1-100?
    //       'u32 uid' or 'struct rico_uid *uid'?
    // Initialize free list
    for (u32 i = 0; i < count; i++)
    {
        pool->handles[i] = i + 1;
    }

#if RICO_DEBUG_POOL
    printf("[pool][init] name=%s\n", pool->uid.name);
#endif

    return SUCCESS;
}

void pool_free(struct rico_pool *pool, destructor *destruct)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(pool->uid.uid != UID_NULL);

#if RICO_DEBUG_POOL
    printf("[pool][free] uid=%d name=%s\n", pool->uid.uid, pool->uid.name);
#endif

    // DEBUG: Make sure contents of pool have been properly freed
    for (u32 i = 0; i < pool->active; ++i)
    {
        u32 handle = pool->handles[i];
        struct rico_uid *uid = pool_read(pool, handle);

        if (uid->uid != UID_NULL)
        {
            destruct(handle);
        }
    }

    free(pool->handles);
    free(pool->data);

    pool->uid.uid = UID_NULL;
}

int pool_handle_alloc(struct rico_pool **pool_ptr, u32 *_handle)
{
    enum rico_error err = SUCCESS;

    struct rico_pool *pool = *pool_ptr;
    RICO_ASSERT(pool);

    ////////////////////////////////////////////////////////////////////////////
    /*
    if (pool->active == pool->count)
    {
        // Resize pool
        struct rico_pool *new_pool;
        err = pool_init(pool->uid.name, pool->count * 2, pool->size,
                        pool->static_count, &new_pool);
        if (err)
        {
#if RICO_DEBUG_POOL
            printf("[pool][ ERR] %s out of memory. Exceeded pool size [%d]. ",
                   pool->uid.name, pool->count);
            pool_print_handles(pool);
#endif
            
            return RICO_FATAL(ERR_POOL_OUT_OF_MEMORY, "%s pool is full",
                              pool->uid.name);
        }

        memcpy(new_pool->handles, pool->handles,
               sizeof(pool->handles[0]) * pool->count);
        memcpy(new_pool->data, pool->data, pool->size * pool->active);
        new_pool->active = pool->active;

        //pool_print_handles(pool);
        //pool_print_handles(new_pool);

        *pool_ptr = new_pool;
        pool = *pool_ptr;

        //pool_print_handles(pool);
        //pool_print_handles(new_pool);
    }
    */
    ////////////////////////////////////////////////////////////////////////////
    if (pool->active == pool->count)
    {
#if RICO_DEBUG_POOL
        printf("[pool][ ERR] %s out of memory. Exceeded pool size [%d]. ",
               pool->uid.name, pool->count);
        pool_print_handles(pool);
#endif

        return RICO_FATAL(ERR_POOL_OUT_OF_MEMORY, "%s pool is full",
                          pool->uid.name);
    }
    ////////////////////////////////////////////////////////////////////////////

    *_handle = pool->handles[pool->active];
    pool->active++;

#if RICO_DEBUG_POOL
    printf("[pool][aloc] uid=%d name=%s handle=%d ", pool->uid.uid,
           pool->uid.name, *_handle);
    pool_print_handles(pool);
#endif

    return err;
}

int pool_handle_free(struct rico_pool *pool, u32 handle)
{
    RICO_ASSERT(pool);

    // Find requested handle
    u32 i;
    for (i = 0; i < pool->active; ++i)
    {
        if (pool->handles[i] == handle)
            break;
    }

    struct rico_uid *uid = pool_read(pool, pool->handles[i]);
    uid->uid = UID_NULL;

    // Move deleted handle to free list
    // E.g. [0 1 2 3 4|5 6 7 8 9]
    //      Free handle 1 (swaps with 4)
    //      [0 4 2 3|1 5 6 7 8 9]

    // Don't rearrange the static sub-pool
    if (i >= pool->static_count)
    {
        pool->active--;
        if (pool->active > 0)
        {
            pool->handles[i] = pool->handles[pool->active];
            pool->handles[pool->active] = handle;
        }
    }

#if RICO_DEBUG_POOL
    printf("[pool][free] uid=%d name=%s handle=%d ", pool->uid.uid,
           pool->uid.name, handle);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}

u32 pool_handle_first(struct rico_pool *pool)
{
    if (pool->active > 0)
        return pool->handles[0];
    else
        return 0;
}

u32 pool_handle_next(struct rico_pool *pool, u32 handle)
{
    if (!handle)
    {
        // Nothing selected, return first item in pool or 0 if empty
        return (pool->active) ? pool->handles[0] : 0;
    }

    for (u32 i = 0; i < pool->active; ++i)
    {
        if (pool->handles[i] == handle)
        {
            if (i + 1 < pool->active)
                return pool->handles[i + 1];
            else
                return pool->handles[0];
        }
    }
    return 0;
}

u32 pool_handle_prev(struct rico_pool *pool, u32 handle)
{
    if (!handle)
    {
        // Nothing selected, return last item in pool or 0 if empty
        return (pool->active) ? pool->handles[pool->active - 1] : 0;
    }

    for (u32 i = 0; i < pool->active; ++i)
    {
        if (pool->handles[i] == handle)
        {
            if (i > 0)
                return pool->handles[i - 1];
            else
                return pool->handles[pool->active - 1];
        }
    }
    return 0;
}

//int pool_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(pool_serialize_0)
{
    enum rico_error err;

    const struct rico_pool *pool = handle;
    fwrite(&pool->count,        sizeof(pool->count),        1, file->fs);
    fwrite(&pool->size,         sizeof(pool->size),         1, file->fs);
    fwrite(&pool->static_count, sizeof(pool->static_count), 1, file->fs);
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
    fread(&pool->static_count, sizeof(pool->static_count), 1, file->fs);
    fread(&pool->active,       sizeof(pool->active),       1, file->fs);

    if(pool->count > 0)
    {
        pool->handles = calloc(pool->count, sizeof(u32));
        if (!pool->handles)
            return RICO_ERROR(ERR_BAD_ALLOC,
                              "Failed to alloc handles for pool %s",
                              pool->uid.name);

        pool->data = calloc(pool->count, pool->size);
        if (!pool->data)
            return RICO_ERROR(ERR_BAD_ALLOC, "Failed to alloc data for pool %s",
                              pool->uid.name);

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
            u32 handle;
            for (u32 i = pool->active - 1; i > pool->static_count; --i)
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
    printf("[pool][load] uid=%d name=%s filename=%s ", pool->uid.uid,
           pool->uid.name, file->filename);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}

static void pool_print_handles(struct rico_pool *pool)
{
#if RICO_DEBUG_POOL
    // if (pool->uid.uid > 1) {
    //     printf("\n");
    //     return;
    // }

    printf("status=");

    if (pool->active == 0)
    {
        printf("[EMPTY]\n");
        return;
    }

    // Print static sub-pool
    if (pool->static_count > 0)
    {
        printf("STC[ ");
        for (u32 i = 0; i < pool->static_count; i++)
        {
            printf("%d ", pool->handles[i]);
        }
        printf("] ");
    }

    // Print dynamic sub-pool
    printf("DYN[ ");
    for (u32 i = pool->static_count; i < pool->active; i++)
    {
        printf("%d ", pool->handles[i]);
    }
    printf("| ");
    for (u32 i = pool->active; i < pool->count; i++)
    {
        printf("%d ", pool->handles[i]);
    }
    printf("]\n");
#else
    UNUSED(pool);
#endif
}