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
int pool_save(const struct rico_pool *pool, FILE *fs);
int pool_load(FILE *fs, struct rico_pool *_pool);

static inline void *pool_read(struct rico_pool *pool, uint32 handle)
{
    rico_assert(pool);
    rico_assert(handle < pool->count);
    return (void *)&(((char *)pool->pool)[pool->stride * handle]);
}

#endif // RICO_POOL_H