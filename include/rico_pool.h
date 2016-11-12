#ifndef RICO_POOL_H
#define RICO_POOL_H

#include "const.h"
#include "rico_uid.h"

struct rico_pool {
    struct rico_uid uid;
    uint32 count;
    uint32 size;
    void *pool;
    uint32 *handles;
    uint32 active;
};

int pool_init(const char *name, uint32 count, uint32 size,
              struct rico_pool *_pool);
int pool_alloc(struct rico_pool *pool, uint32 *_handle);
int pool_free(struct rico_pool *pool, uint32 *handle);

static inline void *pool_read(struct rico_pool *pool, uint32 handle)
{
    rico_assert(pool);
    rico_assert(handle < pool->count);
    return (void *)&(((char *)pool->pool)[pool->size * handle]);
}

static inline uint32 pool_next(struct rico_pool *pool, uint32 handle)
{
    return (handle < pool->active) ? ++handle : 0;
}

static inline uint32 pool_prev(struct rico_pool *pool, uint32 handle)
{
    return (handle > 0) ? --handle : pool->active - 1;
}

#endif // RICO_POOL_H