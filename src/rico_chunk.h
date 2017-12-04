#ifndef RICO_CHUNK_H
#define RICO_CHUNK_H

typedef u32 chunk_pool_counts[POOL_COUNT]; //[PERSIST_COUNT];

struct rico_chunk {
    struct hnd hnd;
    u32 total_size;
    u32 cereal_size;
    chunk_pool_counts pool_counts;

    // TODO: Replace with:
    // mem_arena persistent;
    // mem_arena transient;
    // mem_arena_save(&persistent);
    // mem_arena_load(&persistent, "chunk.bin");
    // mem_arena_push(mem_pool_init(HND_MESH, 30));
    struct rico_pool *pools[PERSIST_COUNT][POOL_COUNT];
};

int chunk_init(struct rico_chunk **_chunk, const char *name,
               const chunk_pool_counts *pool_counts);
void chunk_active_set(struct rico_chunk *chunk);
struct rico_chunk *chunk_active();
SERIAL(chunk_serialize_0);
DESERIAL(chunk_deserialize_0);

static inline struct rico_pool **chunk_pool(struct rico_chunk *chunk,
                                            enum rico_persist persist,
                                            enum rico_pool_item_type type)
{
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->pools[persist][type]);
    return &chunk->pools[persist][type];
}

#endif //RICO_CHUNK_H
