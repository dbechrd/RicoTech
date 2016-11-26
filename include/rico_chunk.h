#ifndef RICO_CHUNK
#define RICO_CHUNK

#include "rico_uid.h"
#include "rico_pool.h"

struct rico_chunk {
    struct rico_uid uid;
    uint32 tex_count;
    uint32 mesh_count;
    uint32 obj_count;
    struct rico_pool textures;
    struct rico_pool meshes;
    struct rico_pool objects;
};

int chunk_init(const char *name, uint32 tex_count, uint32 mesh_count,
               uint32 obj_count, struct rico_pool *textures,
               struct rico_pool *meshes, struct rico_pool *objects,
               struct rico_chunk *_chunk);
int chunk_save(const char *filename, const struct rico_chunk *chunk);
int chunk_load(const char *filename, struct rico_chunk *_chunk);

#endif //RICO_CHUNK