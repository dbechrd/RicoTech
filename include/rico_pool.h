#ifndef RICO_POOL_H
#define RICO_POOL_H

#include "rico_uid.h"

typedef void(destructor)(u32 handle);

struct rico_pool {
    struct rico_uid uid;
    u32 count;          // number of elements
    u32 size;           // size of each element
    u32 static_count;   // number of static elements
    u32 active;         // number of elements in use
    u32 *handles;       // pool handles
    void *data;         // element pool
};

#define POOL_SIZE_HANDLES(count) (count * sizeof(u32))
#define POOL_SIZE_DATA(count, size) (count * size)
#define POOL_SIZE(count, size) (POOL_SIZE_HANDLES(count) + POOL_SIZE_DATA(count, size))

#define POOL_OFFSET_HANDLES() (sizeof(struct rico_pool))
#define POOL_OFFSET_DATA(count) (POOL_OFFSET_HANDLES() + POOL_SIZE_HANDLES(count))

int pool_init(void *mem_block, const char *name, u32 count, u32 size,
              u32 static_count);
void pool_free(struct rico_pool *pool, destructor *destruct);
int pool_handle_alloc(struct rico_pool **pool_ptr, u32 *_handle);
int pool_handle_free(struct rico_pool *pool, u32 handle);
u32 pool_handle_first(struct rico_pool *pool);
u32 pool_handle_next(struct rico_pool *pool, u32 handle);
u32 pool_handle_prev(struct rico_pool *pool, u32 handle);
SERIAL(pool_serialize_0);
DESERIAL(pool_deserialize_0);

static inline void *pool_read(const struct rico_pool *pool, u32 handle)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(handle > 0);
    RICO_ASSERT(handle <= pool->count);

    // Note: Handles are index + 1, 0 is reserved
    return (void *)&(((char *)pool->data)[pool->size * (handle - 1)]);
}

#endif // RICO_POOL_H