#ifndef RICO_POOL_H
#define RICO_POOL_H

#include "const.h"
#include "rico_uid.h"
#include <stdio.h>

struct rico_pool {
    struct rico_uid uid;
    u32 count;        // number of elements
    u32 size;         // size of each element
    u32 static_count; // number of static elements
    u32 active;       // number of elements in use
    u32 *handles;     // pool handles
    void *pool;          // element pool
};

int pool_init(const char *name, u32 count, u32 size, u32 static_count,
              struct rico_pool *_pool);
int pool_alloc(struct rico_pool *pool, u32 *_handle);
int pool_free(struct rico_pool *pool, u32 handle);
u32 pool_next(struct rico_pool *pool, u32 handle);
u32 pool_prev(struct rico_pool *pool, u32 handle);
int pool_serialize_0(const void *handle, const struct rico_file *file);
int pool_deserialize_0(void *handle, const struct rico_file *file);

static inline void *pool_read(const struct rico_pool *pool, u32 handle)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(handle > 0);
    RICO_ASSERT(handle <= pool->count);

    // Note: Handles are index + 1, 0 is reserved
    return (void *)&(((char *)pool->pool)[pool->size * (handle - 1)]);
}

#endif // RICO_POOL_H