#ifndef RICO_CHUNK_H
#define RICO_CHUNK_H

struct rico_chunk {
    struct rico_uid uid;
    u32 total_size;
    u32 count_strings;
    u32 count_fonts;
    u32 count_textures;
    u32 count_materials;
    u32 count_meshes;
    u32 count_objects;
    struct rico_pool *strings;
    struct rico_pool *fonts;
    struct rico_pool *textures;
    struct rico_pool *materials;
    struct rico_pool *meshes;
    struct rico_pool *objects;
};

int chunk_init(const char *name, u32 strings, u32 fonts, u32 textures,
               u32 materials, u32 meshes, u32 objects,
               struct rico_chunk **_chunk);
void chunk_active_set(struct rico_chunk *chunk);
struct rico_chunk *chunk_active();
SERIAL(chunk_serialize_0);
DESERIAL(chunk_deserialize_0);

#endif //RICO_CHUNK_H