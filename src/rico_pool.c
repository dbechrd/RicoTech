#include "rico_pool.h"
#include "rico_object.h"
#include "stdlib.h"
#include "stdio.h"

static void pool_print_handles(struct rico_pool *pool);

// TODO: Allocate from heap pool, don't keep calling calloc
int pool_init(const char *name, uint32 count, uint32 stride,
              struct rico_pool *_pool)
{
    rico_assert(count > 0);

    struct rico_pool pool;
    uid_init(&pool.uid, RICO_UID_POOL, name);
    pool.count = count;
    pool.stride = stride;
    pool.active = 0;

    pool.handles = calloc(count, sizeof(uint32));
    if (!pool.handles) return ERR_BAD_ALLOC;

    pool.pool = calloc(count, stride);
    if (!pool.pool) return ERR_BAD_ALLOC;

    // Initialize free list
    for (uint32 i = 0; i < count; i++)
    {
        pool.handles[i] = i;
    }

#ifdef RICO_DEBUG_POOL
    printf("[Pool %d][%s] Initialized\n", pool.uid.uid, pool.uid.name);
#endif

    *_pool = pool;
    return SUCCESS;
}

int pool_alloc(struct rico_pool *pool, uint32 *_handle)
{
    rico_assert(pool);
    if (pool->active == pool->count)
    {
#ifdef RICO_DEBUG_POOL
        printf("[Pool %d][%s] Out of memory. Exceeded max elements [%d].",
                pool->uid.uid, pool->uid.name, pool->count);
        pool_print_handles(pool);
#endif

        return ERR_POOL_OUT_OF_MEMORY;
    }

    *_handle = pool->handles[pool->active];
    pool->active++;

#ifdef RICO_DEBUG_POOL
    printf("[Pool %d][%s] Alloc handle: %d", pool->uid.uid, pool->uid.name,
           *_handle);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}

int pool_free(struct rico_pool *pool, uint32 handle)
{
    rico_assert(pool);

    // Find requested handle
    uint32 i;
    for (i = 0; i < pool->active; ++i)
    {
        if (pool->handles[i] == handle)
            break;
    }

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

#ifdef RICO_DEBUG_POOL
    printf("[Pool %d][%s] Free handle: %d", pool->uid.uid, pool->uid.name,
           handle);
    pool_print_handles(pool);
#endif

    return SUCCESS;
}

uint32 pool_next(struct rico_pool *pool, uint32 handle)
{
    rico_assert(pool->active);
    for (uint32 i = 0; i < pool->active; ++i)
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

uint32 pool_prev(struct rico_pool *pool, uint32 handle)
{
    rico_assert(pool->active);
    for (uint32 i = 0; i < pool->active; ++i)
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

int pool_serialize(const struct rico_pool *pool, FILE *fs)
{
    // Header
    fwrite(&pool->uid,    sizeof(pool->uid),    1, fs);
    fwrite(&pool->count,  sizeof(pool->count),  1, fs);
    fwrite(&pool->stride, sizeof(pool->stride), 1, fs);
    fwrite(&pool->active, sizeof(pool->active), 1, fs);

    // Data
    fwrite(pool->handles, sizeof(*pool->handles), pool->count, fs);
    //fwrite(pool->pool,    pool->stride,           pool->count, fs);
    for (uint32 i = 0; i < pool->active; ++i)
    {
        enum rico_uid_type *type =
            (enum rico_uid_type *)pool_read(pool, pool->handles[i]);
        if (!RicoSerializers[*type])
            break;

        RicoSerializers[*type](pool->handles[i], fs);
    }

    return SUCCESS;
}

int pool_deserialize(struct rico_pool *_pool, FILE *fs)
{
    struct rico_pool pool = { 0 };

    // Header
    fread(&pool.uid,    sizeof(pool.uid),    1, fs);
    fread(&pool.count,  sizeof(pool.count),  1, fs);
    fread(&pool.stride, sizeof(pool.stride), 1, fs);
    fread(&pool.active, sizeof(pool.active), 1, fs);

    // Data
    if(pool.count > 0)
    {
        pool.handles = calloc(pool.count, sizeof(uint32));
        if (!pool.handles) return ERR_BAD_ALLOC;

        pool.pool = calloc(pool.count, pool.stride);
        if (!pool.pool) return ERR_BAD_ALLOC;

        fread(pool.handles, sizeof(*pool.handles), pool.count, fs);
        //fread(pool.pool,    pool.stride,           pool.count, fs);
        for (uint32 i = 0; i < pool.active; ++i)
        {
            enum rico_uid_type *type =
                (enum rico_uid_type *)pool_read(&pool, pool.handles[i]);
            if (!RicoDeserializers[*type])
                break;

            RicoDeserializers[*type](&pool.handles[i], &pool, fs);
        }
    }

    #ifdef RICO_DEBUG_POOL
        printf("[Pool %d][%s] Loaded from file.", pool.uid.uid, pool.uid.name);
        pool_print_handles(&pool);
    #endif

    *_pool = pool;
    return SUCCESS;
}

#ifdef RICO_DEBUG_POOL
static void pool_print_handles(struct rico_pool *pool)
{
    printf(" Handles: ");

    if (pool->active == 0)
    {
        printf("[EMPTY]\n");
        return;
    }

    printf("[");
    for (uint32 i = 0; i < pool->active; i++)
    {
        printf("%d ", pool->handles[i]);
    }
    printf("\b][");
    for (uint32 i = pool->active; i < pool->count; i++)
    {
        printf("%d ", pool->handles[i]);
    }
    printf("\b]\n");
}
#endif