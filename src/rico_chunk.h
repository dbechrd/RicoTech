#ifndef RICO_CHUNK_H
#define RICO_CHUNK_H

typedef u32 chunk_pool_counts[POOL_COUNT]; //[PERSIST_COUNT];

struct rico_chunk {
    struct rico_uid uid;
    u32 total_size;
    u32 cereal_size;
    chunk_pool_counts pool_counts;
    struct rico_pool *pools[PERSIST_COUNT][POOL_COUNT];
};

int chunk_init(struct rico_chunk **_chunk, const char *name,
               const chunk_pool_counts *pool_counts);
void chunk_active_set(struct rico_chunk *chunk);
struct rico_chunk *chunk_active();
SERIAL(chunk_serialize_0);
DESERIAL(chunk_deserialize_0);

#endif //RICO_CHUNK_H