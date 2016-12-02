#ifndef RICO_POOL_H
#define RICO_POOL_H

#include "const.h"
#include "rico_uid.h"
#include <stdio.h>

struct rico_pool {
    struct rico_uid uid;
    uint32 count;        // number of elements
    uint32 stride;       // size of each element
    uint32 active;       // number of elements in use
    uint32 *handles;     // pool handles
    void *pool;          // element pool
};

int pool_init(const char *name, uint32 count, uint32 stride,
              struct rico_pool *_pool);
int pool_alloc(struct rico_pool *pool, uint32 *_handle);
int pool_free(struct rico_pool *pool, uint32 handle);
uint32 pool_next(struct rico_pool *pool, uint32 handle);
uint32 pool_prev(struct rico_pool *pool, uint32 handle);
int pool_serialize(const void *handle, FILE *fs);
int pool_deserialize(void *handle, FILE *fs);

static inline void *pool_read(const struct rico_pool *pool, uint32 handle)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(handle < pool->count);
    return (void *)&(((char *)pool->pool)[pool->stride * handle]);
}

#endif // RICO_POOL_H