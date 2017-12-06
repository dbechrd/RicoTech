#if 0
const char *rico_persist_string[] = {
    RICO_PERSIST_TYPES(GEN_STRING)
};
const char *rico_pool_itemtype_string[] = {
    RICO_POOL_ITEMTYPES(GEN_STRING)
};

u32 pool_item_sizes[POOL_COUNT] = {
    sizeof(struct rico_string),
    sizeof(struct rico_font),
    sizeof(struct rico_texture),
    sizeof(struct rico_material),
    sizeof(struct rico_mesh),
    sizeof(struct rico_object)
};

u32 pool_item_fixed_counts[POOL_COUNT] = {
    STR_SLOT_DYNAMIC,
    0,
    0,
    0,
    0,
    0
};
#endif

internal void pool_print_handles(struct rico_pool *pool);

// TODO: Allocate from heap pool, don't keep calling calloc
int pool_init(void *mem_block, const char *name, u32 count, u32 size,
              u32 fixed_count)
{
    // TODO: Disallow NULL pools (abusing this for debugging bigchunk atm)
    //RICO_ASSERT(count > 0);
    //RICO_ASSERT(size > 0);

    struct rico_pool *pool = mem_block;
    uid_init(&pool->hnd, RICO_UID_POOL, name, true);
    pool->count = count;
    pool->size = size;
    pool->fixed_count = fixed_count;
    pool->active = fixed_count;

    // Pointer fix-up
    pool_fixup(pool);

    // TODO: Use (struct *hnd) for handles instead of e.g. 1-100?
    // TODO: If handles are pointers directly to object, do we even need
    //       indexed pools anymore?? I don't think so.. we can just return
    //       the next available handle which is an already allocated object.
    // TODO: E.g. pool_init(RICO_HND_MESH, 30)
    // Initialize free list
    for (u32 i = 0; i < count; i++)
    {
        pool->handles[i].persist = persist;
        pool->handles[i].value = i + 1;
    }

#if RICO_DEBUG_POOL
    printf("[pool][init] name=%s\n", pool->hnd.name);
#endif

    return SUCCESS;
}

void pool_free(struct rico_pool *pool, destructor *destruct)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(pool->hnd.uid != UID_NULL);

#if RICO_DEBUG_POOL
    printf("[pool][free] uid=%d name=%s\n", pool->hnd.uid, pool->hnd.name);
#endif

    // DEBUG: Make sure contents of pool have been properly freed
    for (u32 i = 0; i < pool->active; ++i)
    {
        struct hnd *handle = pool->handles[i];
        struct rico_uid *uid = pool_read(pool, handle.value);

        if (uid->uid != UID_NULL)
        {
            destruct(handle);
        }
    }

    free(pool->handles);
    free(pool->data);

    pool->hnd.uid = UID_NULL;
}

int pool_handle_alloc(struct rico_pool *pool, struct hnd **_handle);
{
    RICO_ASSERT(pool);

    enum rico_error err = SUCCESS;

    ////////////////////////////////////////////////////////////////////////////
    // NOTE: Cannot auto-extend pool without updating the corresponding counts
    //       in the chunk.
    ////////////////////////////////////////////////////////////////////////////
    /*
    if (pool->active == pool->count)
    {
        // Resize pool
        struct rico_pool *new_pool;
        err = pool_init(pool->hnd.name, pool->count * 2, pool->size,
                        pool->fixed_count, &new_pool);
        if (err)
        {
#if RICO_DEBUG_POOL
            printf("[pool][ ERR] %s out of memory. Exceeded pool size [%d]. ",
                   pool->hnd.name, pool->count);
            pool_print_handles(pool);
#endif

            return RICO_FATAL(ERR_POOL_OUT_OF_MEMORY, "%s pool is full",
                              pool->hnd.name);
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
               pool->hnd.name, pool->count);
        pool_print_handles(pool);
#endif

        return RICO_FATAL(ERR_POOL_OUT_OF_MEMORY, "%s pool is full",
                          pool->hnd.name);
    }
    ////////////////////////////////////////////////////////////////////////////

    *_handle = pool->handles[pool->active++];

#if RICO_DEBUG_POOL
    printf("[pool][aloc] uid=%d name=%s handle=%d ", pool->hnd.uid,
           pool->hnd.name, _handle->value);
    pool_print_handles(pool);
#endif

    return err;
}

int pool_handle_free(struct rico_pool *pool, struct hnd *handle)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(handle.value);

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i].value != handle.value && i++ < pool->active) {}

    if (pool->handles[i].value != handle.value)
        return RICO_ERROR(ERR_POOL_INVALID_HANDLE, "Pool handle not found.");

    struct rico_uid *uid = pool_read(pool, handle.value);
    uid->uid = UID_NULL;

    // Move deleted handle to free list
    // E.g. [0 1 2 3 4|5 6 7 8 9]
    //      Free handle 1 (swaps with 4)
    //      [0 4 2 3|1 5 6 7 8 9]

    // Don't rearrange the fixed sub-pool
    if (handle.value > pool->fixed_count)
    {
        pool->active--;
        if (pool->active > 0)
        {
            pool->handles[i] = pool->handles[pool->active];
            pool->handles[pool->active] = handle;
        }
    }

#if RICO_DEBUG_POOL
    printf("[pool][free] uid=%d name=%s handle=%d ", pool->hnd.uid,
           pool->hnd.name, handle.value);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}

struct hnd *pool_handle_first(struct rico_pool *pool)
{
    // Pool is empty, return null
    if (pool->active == 0)
        return HANDLE_NULL;

    return pool->handles[0];
}

struct hnd *pool_handle_next(struct rico_pool *pool, struct hnd *handle)
{
    RICO_ASSERT(pool);

    // Pool is empty, return null
    if (pool->active == 0)
        return HANDLE_NULL;

    // Nothing or last item selected, return first item in pool
    if (!handle.value || handle.value >= pool->active)
        return pool->handles[0];

    // Return next item
    return pool->handles[handle.value];
}

struct hnd *pool_handle_prev(struct rico_pool *pool, struct hnd *handle)
{
    RICO_ASSERT(pool);

    // Pool is empty, return null
    if (pool->active == 0)
        return HANDLE_NULL;

    // Nothing or first item selected, return last item in pool
    if (handle.value <= 1)
        return pool->handles[pool->active - 1];

    // Return previous item
    return pool->handles[handle.value - 2];
}

#if 0
//int pool_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(pool_serialize_0)
{
    enum rico_error err;

    const struct rico_pool *pool = handle;
    fwrite(&pool->count,        sizeof(pool->count),        1, file->fs);
    fwrite(&pool->size,         sizeof(pool->size),         1, file->fs);
    fwrite(&pool->fixed_count, sizeof(pool->fixed_count), 1, file->fs);
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
    fread(&pool->fixed_count, sizeof(pool->fixed_count), 1, file->fs);
    fread(&pool->active,       sizeof(pool->active),       1, file->fs);

    if(pool->count > 0)
    {
        pool->handles = calloc(pool->count, sizeof(u32));
        if (!pool->handles)
            return RICO_ERROR(ERR_BAD_ALLOC,
                              "Failed to alloc handles for pool %s",
                              pool->hnd.name);

        pool->data = calloc(pool->count, pool->size);
        if (!pool->data)
            return RICO_ERROR(ERR_BAD_ALLOC, "Failed to alloc data for pool %s",
                              pool->hnd.name);

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
            for (u32 i = pool->active - 1; i > pool->fixed_count; --i)
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
    printf("[pool][load] uid=%d name=%s filename=%s ", pool->hnd.uid,
           pool->hnd.name, file->filename);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}
#endif

internal void pool_print_handles(struct rico_pool *pool)
{
#if RICO_DEBUG_POOL
    // if (pool->hnd.uid > 1) {
    //     printf("\n");
    //     return;
    // }

    printf("status= ");

    if (pool->active == 0)
    {
        printf("[EMPTY]\n");
        return;
    }

    // Print fixed sub-pool
    if (pool->fixed_count > 0)
    {
        printf("FIXED: ");
        for (u32 i = 0; i < pool->fixed_count; i++)
        {
            printf("%d ", pool->handles[i].value);
        }
    }

    // Print dynamic sub-pool
    printf("DYNAMIC: ");
    for (u32 i = pool->fixed_count; i < pool->active; i++)
    {
        printf("%d ", pool->handles[i].value);
    }
    printf("FREE: ");
    for (u32 i = pool->active; i < pool->count; i++)
    {
        printf("%d ", pool->handles[i].value);
    }
    printf("]\n");
#else
    UNUSED(pool);
#endif
}
