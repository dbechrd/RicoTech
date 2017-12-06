#ifndef RICO_CHUNK_H
#define RICO_CHUNK_H

#define CHUNK_POOL_COUNT RICO_HND_CEREAL_COUNT
typedef u32 chunk_pool_counts[CHUNK_POOL_COUNT];

// TODO: Make this a global hash table (RICO_HND_TYPE -> u32 size)
u32 size_by_handle[RICO_HND_CEREAL_COUNT] = {

};

struct rico_chunk {
    struct hnd hnd;
    u32 total_size;
    u32 cereal_size;
    chunk_pool_counts pool_counts;

    // TODO: All pools in chunk are persistent, transient pools should exist
    //       solely in a memory arena.
    // mem_arena persistent;
    // mem_arena transient;
    // mem_arena_save(&persistent);
    // mem_arena_load(&persistent, "chunk.bin");
    // mem_arena_push(mem_pool_init(HND_MESH, 30));
    struct rico_pool *pools[CHUNK_POOL_COUNT];
};
global struct rico_chunk *chunk_active;
global struct rico_chunk *chunk_transient;

int chunk_init(struct rico_chunk **_chunk, const char *name,
               const chunk_pool_counts *pool_counts);
int chunk_alloc(struct rico_chunk *chunk, enum rico_hnd_type type,
                struct hnd **_handle);
SERIAL(chunk_serialize_0);
DESERIAL(chunk_deserialize_0);

static inline struct rico_pool *chunk_pool(struct rico_chunk *chunk,
                                           enum rico_hnd_type type)
{
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->pools[type]);
    return chunk->pools[type];
}

#endif //RICO_CHUNK_H
