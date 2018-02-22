#ifndef RICO_INTERNAL_POOL_H
#define RICO_INTERNAL_POOL_H

struct hnd;
struct rico_chunk;

// TODO: Pack this
// mark, tag, badge, pin, peg
struct pool_id
{
    enum RICO_hnd_type type;
    u32 tag;
    u32 generation;
    u32 pool_uid;
};
struct pool_id ID_NULL = { 0 };

struct pool_tag
{
    u32 block;
    u32 generation;
    //u32 ref_count;
    u32 next_free;
};

struct rico_pool
{
    u32 uid;
    char name[32];
    u32 block_count;
    u32 block_size;
    u32 blocks_used;
    u32 next_free;
    u32 *block_tags;
    struct pool_tag *tags;
    u8 *blocks;
    u8 *end;
};

#define POOL_BLOCK_TAGS_SIZE(block_count) \
    (block_count * sizeof(u32))
#define POOL_TAGS_SIZE(block_count) \
    (block_count * sizeof(struct pool_tag))
#define POOL_BLOCKS_SIZE(block_count, block_size) \
    (block_count * block_size)
#define POOL_SIZE(block_count, block_size) \
    (sizeof(struct rico_pool) + \
    POOL_BLOCK_TAGS_SIZE(block_count) + \
    POOL_TAGS_SIZE(block_count) + \
    POOL_BLOCKS_SIZE(block_count, block_size))

#define POOL_BLOCK_TAGS_OFFSET() \
    (sizeof(struct rico_pool))
#define POOL_TAGS_OFFSET(block_count) \
    (POOL_BLOCK_TAGS_OFFSET() + \
    POOL_BLOCK_TAGS_SIZE(block_count))
#define POOL_BLOCKS_OFFSET(block_count) \
    (POOL_TAGS_OFFSET(block_count) + \
    POOL_TAGS_SIZE(block_count))

extern inline void pool_fixup(struct rico_pool *pool, struct rico_chunk *chunk);
int pool_init(void *buf, const char *name, u32 block_count, u32 block_size);
int pool_add(struct hnd **handle, struct rico_pool *pool);
int pool_remove(struct rico_pool *pool, struct pool_id id);
extern inline void *pool_first(struct rico_pool *pool);
extern inline void *pool_last(struct rico_pool *pool);
extern inline void *pool_next(struct rico_pool *pool, void *block);
extern inline void *pool_prev(struct rico_pool *pool, void *block);
extern inline void *pool_read(struct rico_pool *pool, struct pool_id id);
extern inline struct pool_id pool_next_id(struct rico_pool *pool,
                                          struct pool_id id);
extern inline struct pool_id pool_prev_id(struct rico_pool *pool,
                                          struct pool_id id);
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

#endif
