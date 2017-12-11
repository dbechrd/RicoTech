#ifndef RICO_POOL_H
#define RICO_POOL_H

//#include "rico_uid.h"

struct rico_pool {
    struct hnd hnd;
    u32 count;            // number of elements
    u32 size;             // size of each element
    u32 active;           // number of elements in use
    struct hnd **handles; // pool handles
    u8 *data;             // element pool
};

typedef void(destructor)(struct hnd *handle);

#define POOL_SIZE_HANDLES(count) (count * sizeof(struct hnd *))
#define POOL_SIZE_DATA(count, size) (count * size)
#define POOL_SIZE(count, size) (sizeof(struct rico_pool) + \
                                POOL_SIZE_HANDLES(count) + \
                                POOL_SIZE_DATA(count, size))

#define POOL_OFFSET_HANDLES() (sizeof(struct rico_pool))
#define POOL_OFFSET_DATA(count) (POOL_OFFSET_HANDLES() + \
                                 POOL_SIZE_HANDLES(count))

static inline void pool_fixup(struct rico_pool *pool);
static inline void pool_fixup_handles(struct rico_pool *pool,
                                      struct rico_chunk *chunk,
                                      enum rico_hnd_type type);
int pool_init(void *mem_block, const char *name, u32 count, u32 size);
void pool_free(struct rico_pool *pool, destructor *destruct);
int pool_handle_alloc(struct rico_pool *pool, struct hnd **_handle);
int pool_handle_free(struct rico_pool *pool, struct hnd *handle);
struct hnd *pool_handle_first(struct rico_pool *pool);
struct hnd *pool_handle_last(struct rico_pool *pool);
struct hnd *pool_handle_next(struct rico_pool *pool, struct hnd *handle);
struct hnd *pool_handle_prev(struct rico_pool *pool, struct hnd *handle);
//SERIAL(pool_serialize_0);
//DESERIAL(pool_deserialize_0);

#if 0
static inline void *pool_read(const struct rico_pool *pool, u32 value)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(value > 0);
    RICO_ASSERT(value <= pool->count);

    // Note: Handle values start at 1; 0 is reserved
    return (void *)&((pool->data)[pool->size * (value - 1)]);
}
#endif

#endif // RICO_POOL_H
