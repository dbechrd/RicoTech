#include "rico_pool.h"
#include "rico_object.h"
#include "rico_cereal.h"
#include <stdlib.h>
#include <stdio.h>

static void pool_print_handles(struct rico_pool *pool);

// TODO: Allocate from heap pool, don't keep calling calloc
int pool_init(const char *name, u32 count, u32 size, u32 static_count,
              struct rico_pool *_pool)
{
    RICO_ASSERT(count > 0);

    struct rico_pool pool;
    uid_init(&pool.uid, RICO_UID_POOL, name, true);
    pool.count = count;
    pool.size = size;
    pool.static_count = static_count;
    pool.active = static_count;

    pool.handles = calloc(count, sizeof(u32));
    if (!pool.handles) return RICO_ERROR(ERR_BAD_ALLOC);

    pool.pool = calloc(count, size);
    if (!pool.pool) return RICO_ERROR(ERR_BAD_ALLOC);

    // TODO: Should I use UIDs for handles instead of e.g. 1-100?
    //       'u32 uid' or 'struct rico_uid *uid'?
    // Initialize free list
    for (u32 i = 0; i < count; i++)
    {
        pool.handles[i] = i + 1;
    }

#ifdef RICO_DEBUG_POOL
    printf("[pool][init] name=%s\n", pool.uid.name);
#endif

    *_pool = pool;
    return SUCCESS;
}

void pool_free(struct rico_pool *pool, destructor *destruct)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(pool->uid.uid != UID_NULL);

#ifdef RICO_DEBUG_POOL
    printf("[pool][free] name=%s\n", pool->uid.name);
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
    free(pool->pool);

    pool->uid.uid = UID_NULL;
}

int pool_handle_alloc(struct rico_pool *pool, u32 *_handle)
{
    RICO_ASSERT(pool);
    if (pool->active == pool->count)
    {
#ifdef RICO_DEBUG_POOL
        printf("[pool][ ERR] %s out of memory. Exceeded pool size [%d].\n",
               pool->uid.name, pool->count);
        pool_print_handles(pool);
#endif

        return RICO_ERROR(ERR_POOL_OUT_OF_MEMORY);
    }

    *_handle = pool->handles[pool->active];
    pool->active++;

#ifdef RICO_DEBUG_POOL
    printf("[pool][aloc] name=%s handle=%d\n", pool->uid.name, *_handle);
    pool_print_handles(pool);
#endif

    return SUCCESS;
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

#ifdef RICO_DEBUG_POOL
    printf("[pool][free] name=%s handle=%d\n", pool->uid.name, handle);
    pool_print_handles(pool);
#endif

    return SUCCESS;
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

int pool_serialize_0(const void *handle, const struct rico_file *file)
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

int pool_deserialize_0(void *_handle, const struct rico_file *file)
{
    enum rico_error err;

    struct rico_pool *pool = _handle;
    fread(&pool->count,        sizeof(pool->count),        1, file->fs);
    fread(&pool->size,         sizeof(pool->size),         1, file->fs);
    fread(&pool->static_count, sizeof(pool->static_count), 1, file->fs);
    fread(&pool->active,       sizeof(pool->active),       1, file->fs);

    if(pool->count > 0)
    {
        pool->handles = calloc(pool->count, sizeof(u32));
        if (!pool->handles) return RICO_ERROR(ERR_BAD_ALLOC);

        pool->pool = calloc(pool->count, pool->size);
        if (!pool->pool) return RICO_ERROR(ERR_BAD_ALLOC);

        fread(pool->handles, sizeof(*pool->handles), pool->count, file->fs);
        for (u32 i = 0; i < pool->active; ++i)
        {
            err = rico_deserialize(pool_read(pool, pool->handles[i]), file);
            if (err == ERR_SERIALIZE_DISABLED)
                continue;
            if (err)
                return err;
        }

        pool_print_handles(pool);

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

                pool_print_handles(pool);
            }
        }
    }

#ifdef RICO_DEBUG_POOL
    printf("[pool][load] name=%s filename=%s\n", pool->uid.name,
           file->filename);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}

static void pool_print_handles(struct rico_pool *pool)
{
#ifdef RICO_DEBUG_POOL
    // if (pool->uid.uid > 1) {
    //     printf("\n");
    //     return;
    // }

    printf(" status=");

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