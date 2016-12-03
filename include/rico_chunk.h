#ifndef RICO_CHUNK
#define RICO_CHUNK

#include "rico_uid.h"
#include "rico_pool.h"

struct rico_chunk {
    struct rico_uid uid;
    uint32 version;
    uint32 tex_count;
    uint32 mesh_count;
    uint32 obj_count;
    struct rico_pool textures;
    struct rico_pool meshes;
    struct rico_pool objects;
};

int chunk_init(uint32 version, const char *name, uint32 tex_count,
               uint32 mesh_count, uint32 obj_count, struct rico_pool *textures,
               struct rico_pool *meshes, struct rico_pool *objects,
               struct rico_chunk *_chunk);
int chunk_serialize_0(const void *handle, const struct rico_file *file);
int chunk_deserialize_0(void *_handle, const struct rico_file *file);

#endif //RICO_CHUNK