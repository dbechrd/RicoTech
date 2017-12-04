#ifndef RICO_POOL_H
#define RICO_POOL_H

//#include "rico_uid.h"

struct rico_pool {
    struct hnd hnd;
    u32 count;            // number of elements
    u32 size;             // size of each element
    u32 fixed_count;      // number of fixed elements
    u32 active;           // number of elements in use
    struct hnd *handles;  // pool handles
    u8 *data;             // element pool
};

typedef void(destructor)(struct hnd *handle);

#define RICO_PERSIST_TYPES(f)  \
    f(PERSISTENT) \
    f(TRANSIENT)  \
    f(PERSIST_COUNT)

enum rico_persist {
    RICO_PERSIST_TYPES(GEN_LIST)
};
extern const char *rico_persist_string[];

#define RICO_POOL_ITEMTYPES(f) \
    f(POOL_STRINGS)   \
    f(POOL_FONTS)     \
    f(POOL_TEXTURES)  \
    f(POOL_MATERIALS) \
    f(POOL_MESHES)    \
    f(POOL_OBJECTS)   \
    f(POOL_COUNT)

enum rico_pool_item_type {
    RICO_POOL_ITEMTYPES(GEN_LIST)
};
extern const char *rico_pool_itemtype_string[];

extern u32 pool_item_sizes[POOL_COUNT];
extern u32 pool_item_fixed_counts[POOL_COUNT];

#define POOL_SIZE_HANDLES(count) (count * sizeof(u32))
#define POOL_SIZE_DATA(count, size) (count * size)
#define POOL_SIZE(count, size) (sizeof(struct rico_pool) + \
                                POOL_SIZE_HANDLES(count) + \
                                POOL_SIZE_DATA(count, size))

#define POOL_OFFSET_HANDLES() (sizeof(struct rico_pool))
#define POOL_OFFSET_DATA(count) (POOL_OFFSET_HANDLES() + \
                                 POOL_SIZE_HANDLES(count))

int pool_init(void *mem_block, enum rico_persist persist, const char *name,
              u32 count, u32 size, u32 fixed_count);
void pool_free(struct rico_pool *pool, destructor *destruct);
int pool_handle_alloc(struct rico_pool *pool, struct hnd *_handle, void **_item);
int pool_handle_free(struct rico_pool *pool, struct hnd *handle);
struct hnd pool_handle_first(struct rico_pool *pool);
struct hnd pool_handle_next(struct rico_pool *pool, struct hnd *handle);
struct hnd pool_handle_prev(struct rico_pool *pool, struct hnd *handle);
//SERIAL(pool_serialize_0);
//DESERIAL(pool_deserialize_0);

static inline void pool_fixup(struct rico_pool *pool)
{
    // TODO: Could clean this up with PTR_ADD_BYTE macro
    pool->handles = (struct hnd *)((u8 *)pool + POOL_OFFSET_HANDLES());
    pool->data = (u8 *)pool + POOL_OFFSET_DATA(pool->count);
}

static inline void *pool_read(const struct rico_pool *pool, u32 value)
{
    RICO_ASSERT(pool);
    RICO_ASSERT(value > 0);
    RICO_ASSERT(value <= pool->count);

    // Note: Handle values start at 1; 0 is reserved
    return (void *)&((pool->data)[pool->size * (value - 1)]);
}

#endif // RICO_POOL_H
