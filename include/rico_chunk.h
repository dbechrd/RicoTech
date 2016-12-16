#ifndef RICO_CHUNK
#define RICO_CHUNK

#include "rico_uid.h"
#include "rico_pool.h"

struct rico_chunk {
    struct rico_uid uid;
    u32 version;
    u32 tex_count;
    u32 mesh_count;
    u32 obj_count;
    struct rico_pool textures;
    struct rico_pool meshes;
    struct rico_pool objects;
};

int chunk_init(u32 version, const char *name, u32 tex_count,
               u32 mesh_count, u32 obj_count, struct rico_pool *textures,
               struct rico_pool *meshes, struct rico_pool *objects,
               struct rico_chunk *_chunk);
int chunk_serialize_0(const void *handle, const struct rico_file *file);
int chunk_deserialize_0(void *_handle, const struct rico_file *file);

#endif //RICO_CHUNK