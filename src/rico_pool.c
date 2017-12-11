internal void pool_print_handles(struct rico_pool *pool);

static inline void pool_fixup(struct rico_pool *pool)
{
    // TODO: Could clean this up with PTR_ADD_BYTE macro
    pool->handles = (struct hnd **)((u8 *)pool + POOL_OFFSET_HANDLES());
    pool->data = (u8 *)pool + POOL_OFFSET_DATA(pool->count);

    // Fix handle pointers to point to this pool's data block
    u8 *ptr = (u8 *)pool->data;
    for (u32 i = 0; i < pool->count; i++)
    {
        struct hnd *hnd = (struct hnd *)ptr;
        if (i < pool->active)
            hashtable_insert_uid(&global_uids, hnd->uid, hnd);

        pool->handles[i] = hnd;
        ptr += pool->size;
    }
}

static inline void pool_fixup_handles(struct rico_pool *pool,
                                      struct rico_chunk *chunk,
                                      enum rico_hnd_type type)
{
    for (u32 i = 0; i < pool->active; i++)
    {
        struct hnd *hnd = pool->handles[i];
        hnd->chunk = chunk;

        if (chunk)
        {
            switch (type)
            {
            case RICO_HND_OBJECT:
                object_fixup((struct rico_object *)hnd);
                break;
            case RICO_HND_TEXTURE:
                break;
            case RICO_HND_MESH:
                break;
            case RICO_HND_FONT:
                font_fixup((struct rico_font *)hnd);
                break;
            case RICO_HND_STRING:
                string_fixup((struct rico_string *)hnd);
                break;
            case RICO_HND_MATERIAL:
                material_fixup((struct rico_material *)hnd);
                break;
            default:
                break;
            }
        }
    }
}

// TODO: Allocate from heap pool, don't keep calling calloc
int pool_init(void *mem_block, const char *name, u32 count, u32 size)
{
    // TODO: Disallow NULL pools (abusing this for debugging bigchunk atm)
    //RICO_ASSERT(count > 0);
    //RICO_ASSERT(size > 0);

    struct rico_pool *pool = mem_block;
    hnd_init(&pool->hnd, RICO_HND_POOL, name);
    pool->count = count;
    pool->size = size;
    pool->active = 0;

    // Pointer fix-up
    pool_fixup(pool);

#if RICO_DEBUG_POOL
    printf("[pool][init] %s\n", pool->hnd.name);
#endif

    return SUCCESS;
}

void pool_free(struct rico_pool *pool, destructor *destruct)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(pool->hnd.uid != UID_NULL);

#if RICO_DEBUG_POOL
    printf("[pool][free] %s\n", pool->hnd.name);
#endif

    // DEBUG: Make sure contents of pool have been properly freed
    for (u32 i = 0; i < pool->active; ++i)
    {
        struct hnd *hnd = pool->handles[i];
        if (hnd->uid != UID_NULL)
        {
            destruct(hnd);
        }
    }

    free(pool->handles);
    free(pool->data);

    pool->hnd.uid = UID_NULL;
}

#if 0
internal int pool_grow(struct rico_pool *pool)
{
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
}
#endif

int pool_handle_alloc(struct rico_pool *pool, struct hnd **_handle)
{
    RICO_ASSERT(pool);

    enum rico_error err = SUCCESS;

    if (pool->active == pool->count)
    {
#if RICO_DEBUG_POOL
        printf("[pool][ ERR] %s out of memory. Exceeded pool size [%d].\n",
               pool->hnd.name, pool->count);
        pool_print_handles(pool);
#endif

        return RICO_FATAL(ERR_POOL_OUT_OF_MEMORY, "%s pool is full",
                          pool->hnd.name);
    }

    *_handle = pool->handles[pool->active];
    pool->active++;

#if RICO_DEBUG_POOL
    printf("[pool][aloc] %s\n",
           pool->hnd.name);
    pool_print_handles(pool);
#endif

    return err;
}

int pool_handle_free(struct rico_pool *pool, struct hnd *handle)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(handle);

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i] != handle && i < pool->active)
        i++;

    if (pool->handles[i] != handle)
        return RICO_ERROR(ERR_POOL_INVALID_HANDLE, "Pool handle not found.");

    handle->uid = UID_NULL;

    // Move deleted handle to free list
    // E.g. [0 1 2 3 4|5 6 7 8 9]
    //      Free handle 1 (swaps with 4)
    //      [0 4 2 3|1 5 6 7 8 9]
    pool->active--;
    if (pool->active > 0)
    {
        pool->handles[i] = pool->handles[pool->active];
        pool->handles[pool->active] = handle;
    }

#if RICO_DEBUG_POOL
    printf("[pool][free] %s\n",
           pool->hnd.name);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}

struct hnd *pool_handle_first(struct rico_pool *pool)
{
    // Pool is empty, return null
    if (pool->active == 0)
        return NULL;

    return pool->handles[0];
}

struct hnd *pool_handle_last(struct rico_pool *pool)
{
    // Pool is empty, return null
    if (pool->active == 0)
        return NULL;

    return pool->handles[pool->active - 1];
}

struct hnd *pool_handle_next(struct rico_pool *pool, struct hnd *handle)
{
    RICO_ASSERT(pool);

    // Nothing selected, return first item in pool
    if (!handle)
        return pool_handle_first(pool);

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->active) {}

    // Last item selected, return first item in pool
    if (i == pool->active)
        return pool_handle_first(pool);

    // Return next item
    return pool->handles[i];
}

struct hnd *pool_handle_prev(struct rico_pool *pool, struct hnd *handle)
{
    RICO_ASSERT(pool);

    // Nothing selected, return last item in pool
    if (!handle)
        return pool_handle_last(pool);

    // First item selected, return last item in pool
    if (handle->uid == pool->handles[0]->uid)
        return pool->handles[pool->active - 1];

    // Find requested handle
    u32 i = 0;
    while (pool->handles[i]->uid != handle->uid && i++ < pool->active) {}

    // Return previous item
    return pool->handles[i - 1];
}

#if 0
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

    printf("             [");
    if (pool->active == 0)
    {
        printf("EMPTY]\n");
        return;
    }

    // Print pool
    for (u32 i = 0; i < pool->active; i++)
    {
        printf("%p", pool->handles[i]);
        if (i < pool->active - 1)
            printf(" ");
    }
    printf("]\n");
    fflush(stdout);
#else
    UNUSED(pool);
#endif
}
