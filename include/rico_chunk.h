#ifndef RICO_CHUNK
#define RICO_CHUNK

#include "rico_uid.h"
#include "rico_pool.h"

struct rico_chunk {
    struct rico_uid uid;
    u32 version;
    u32 size;
    struct rico_pool *strings;
    struct rico_pool *fonts;
    struct rico_pool *textures;
    struct rico_pool *materials;
    struct rico_pool *meshes;
    struct rico_pool *objects;
};

int chunk_init(const char *name, u32 version, u32 strings, u32 fonts,
               u32 textures, u32 materials, u32 meshes, u32 objects,
               struct rico_chunk *_chunk);
SERIAL(chunk_serialize_0);
DESERIAL(chunk_deserialize_0);

#endif //RICO_CHUNK