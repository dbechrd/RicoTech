#ifndef RICO_POOL_H
#define RICO_POOL_H

// TODO: Pack this
// mark, tag, badge, pin, peg
struct pool_id {
    //enum rico_hnd_type type;
    u32 tag;
    //u32 generation;
};

struct block_tag {
    union {
        u32 next_free;
        u32 block_idx;
    };
    u32 tag_idx;
    u32 ref_count;
};

struct rico_pool {
    char name[32];
    u32 block_count;
    u32 block_size;
    u32 blocks_used;
    u32 next_free;
    struct block_tag *tags;
    u8 *blocks;
};

typedef void(destructor)(struct hnd *handle);

#define POOL_SIZE_HANDLES(block_count) (block_count * sizeof(struct block_tag))
#define POOL_SIZE_DATA(block_count, block_size) (block_count * block_size)
#define POOL_SIZE(block_count, block_size) (sizeof(struct rico_pool) + \
    POOL_SIZE_HANDLES(block_count) + POOL_SIZE_DATA(block_count, block_size))
#define POOL_OFFSET_HANDLES() (sizeof(struct rico_pool))
#define POOL_OFFSET_DATA(block_count) (POOL_OFFSET_HANDLES() + \
                                 POOL_SIZE_HANDLES(block_count))

static inline void pool_fixup(struct rico_pool *pool);
int pool_init(void *buf, const char *name, u32 block_count, u32 block_size);
int pool_add(struct rico_pool *pool, struct pool_id *id);
int pool_remove(struct rico_pool *pool, struct pool_id *id);
//struct pool_hnd *pool_first(struct rico_pool *pool);
//struct pool_hnd *pool_last(struct rico_pool *pool);
//struct pool_hnd *pool_next(struct rico_pool *pool, struct pool_hnd *hnd);
//struct pool_hnd *pool_prev(struct rico_pool *pool, struct pool_hnd *hnd);
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
