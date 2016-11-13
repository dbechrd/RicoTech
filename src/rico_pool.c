#include "rico_pool.h"
#include "stdlib.h"
#include "stdio.h"

static void pool_print(struct rico_pool *pool);

// TODO: Allocate from heap pool, don't keep calling calloc
int pool_init(const char *name, uint32 count, uint32 size,
              struct rico_pool *_pool)
{
    struct rico_pool pool;
    uid_init(name, &pool.uid);
    pool.count = count;
    pool.size = size;
    pool.pool = calloc(count, size);
    pool.handles = calloc(count, sizeof(uint32));
    pool.active = 0;

    if (!pool.pool) return ERR_BAD_ALLOC;
    if (!pool.handles) return ERR_BAD_ALLOC;

    // Initialize free list
    for (uint32 i = 0; i < count; i++)
    {
        pool.handles[i] = i;
    }

#ifdef RICO_DEBUG_INFO
    printf("[Pool %d %s] Initialized\n", pool.uid.uid, pool.uid.name);
    pool_print(&pool);
#endif

    *_pool = pool;
    return SUCCESS;
}

int pool_alloc(struct rico_pool *pool, uint32 *_handle)
{
    rico_assert(pool);
    if (pool->active == pool->count)
    {
        fprintf(stderr, "[Pool %d %s] Out of memory. Exceeded max elements [%d].\n",
                pool->uid.uid, pool->uid.name, pool->count);

        return ERR_POOL_OUT_OF_MEMORY;
    }
    
    *_handle = pool->handles[pool->active];
    pool->active++;

#ifdef RICO_DEBUG_INFO
    printf("[Pool %d %s] Alloc handle: %d\n", pool->uid.uid, pool->uid.name,
           *_handle);
    pool_print(pool);
#endif

    return SUCCESS;
}

int pool_free(struct rico_pool *pool, uint32 *handle)
{
    rico_assert(pool);

    //Reorder handle list
    for (uint32 i = *handle; i <= pool->active; i++)
    {
        pool->handles[i] = pool->handles[i + 1];
    }
    pool->active--;
    pool->handles[pool->active] = *handle;

#ifdef RICO_DEBUG_INFO
    printf("[Pool %d %s] Free handle: %d\n", pool->uid.uid, pool->uid.name,
           *handle);
    pool_print(pool);
#endif

    *handle = 0;
    return SUCCESS;
}

#ifdef RICO_DEBUG_INFO
static void pool_print(struct rico_pool *pool)
{
    printf("[Pool %d %s] Active handles: ", pool->uid.uid, pool->uid.name);

    if (pool->active == 0)
    {
        printf("None\n");
        return;
    }

    printf("[");
    for (uint32 i = 0; i < pool->active; i++)
    {
        printf("%d ", pool->handles[i]);
    }
    printf("\b]\n");
}
#endif