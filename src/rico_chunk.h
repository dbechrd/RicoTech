#ifndef RICO_CHUNK_H
#define RICO_CHUNK_H

typedef u32 chunk_pool_counts[RICO_HND_CEREAL_COUNT];

// TODO: Should this be a global hash table (RICO_HND_TYPE -> u32 size)?
const chunk_pool_counts size_by_handle = {
    0,                            // RICO_HND_NULL
    sizeof(struct rico_object),   // RICO_HND_OBJECT
    sizeof(struct rico_texture),  // RICO_HND_TEXTURE
    sizeof(struct rico_mesh),     // RICO_HND_MESH
    sizeof(struct rico_font),     // RICO_HND_FONT
    sizeof(struct rico_string),   // RICO_HND_STRING
    sizeof(struct rico_material)  // RICO_HND_MATERIAL
};

struct rico_chunk {
    u32 uid;
    char name[32];
    u32 total_size;
    chunk_pool_counts pool_counts;

    // TODO: All pools in chunk are persistent, transient pools should exist
    //       solely in a memory arena.
    // mem_arena persistent;
    // mem_arena transient;
    // mem_arena_save(&persistent);
    // mem_arena_load(&persistent, "chunk.bin");
    // mem_arena_push(mem_pool_init(HND_MESH, 30));

    // TODO: Make this **pools to allow dynamically allocating new pools
    //       as necessary?? Or have a non-pool heap in the chunk?
    struct rico_pool *pools[RICO_HND_CEREAL_COUNT];
};
extern struct rico_chunk *chunk_active;
extern struct rico_chunk *chunk_transient;

int chunk_init(struct rico_chunk **_chunk, const char *name,
               const chunk_pool_counts *pool_counts);
int chunk_alloc(void **block, struct rico_chunk *chunk,
                enum rico_hnd_type type);
int chunk_free(struct rico_chunk *chunk, struct pool_id id);
inline void *chunk_read(struct rico_chunk *chunk, struct pool_id id);
inline struct pool_id chunk_next_id(struct rico_chunk *chunk,
                                    struct pool_id id);
inline struct pool_id chunk_prev_id(struct rico_chunk *chunk,
                                    struct pool_id id);
inline struct pool_id chunk_dupe(struct rico_chunk *chunk, struct pool_id id);
int chunk_serialize(const struct rico_chunk *chunk,
                    const struct rico_file *file);
int chunk_deserialize(struct rico_chunk **_chunk, const struct rico_file *file);

/*
static inline struct rico_pool *chunk_pool(struct rico_chunk *chunk,
                                           enum rico_hnd_type type)
{
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->pools[type]);
    return chunk->pools[type];
}
*/

#endif //RICO_CHUNK_H
