#include "rico_chunk.h"
#include "const.h"
#include "rico_cereal.h"
#include "rico_object.h"
#include "rico_material.h"
#include <stdio.h>

static void chunk_print(struct rico_chunk *chunk);

static u32 parse_mesh(const char *mesh)
{
    UNUSED(mesh);
    return 0;
}

int chunk_init(u32 version, const char *name, u32 tex_count, u32 mat_count,
               u32 mesh_count, u32 obj_count, struct rico_pool *textures,
               struct rico_pool *materials, struct rico_pool *meshes,
               struct rico_pool *objects, struct rico_chunk *_chunk)
{
    UNUSED(tex_count);
    UNUSED(mesh_count);
    UNUSED(textures);
    UNUSED(meshes);

    struct rico_chunk chunk;
    uid_init(&chunk.uid, RICO_UID_CHUNK, name, true);
    chunk.version = version;
    chunk.tex_count = 0; //tex_count;
    chunk.mat_count = mat_count;
    chunk.mesh_count = 0; //mesh_count;
    chunk.obj_count = obj_count;
    chunk.textures = (struct rico_pool) { 0 }; //*textures;
    chunk.materials = *materials;
    chunk.meshes = (struct rico_pool) { 0 }; //*meshes;
    chunk.objects = *objects;

#ifdef RICO_DEBUG_CHUNK
    printf("[chnk][init] name=%s\n", chunk.uid.name);
    chunk_print(&chunk);
#endif

    *_chunk = chunk;
    return SUCCESS;
}

int chunk_serialize_0(const void *handle, const struct rico_file *file)
{
    enum rico_error err;
    const struct rico_chunk *chunk = handle;

    fwrite(&chunk->version,    sizeof(chunk->version),    1, file->fs);
    fwrite(&chunk->tex_count,  sizeof(chunk->tex_count),  1, file->fs);
    fwrite(&chunk->mat_count,  sizeof(chunk->mat_count),  1, file->fs);
    fwrite(&chunk->mesh_count, sizeof(chunk->mesh_count), 1, file->fs);
    fwrite(&chunk->obj_count,  sizeof(chunk->obj_count),  1, file->fs);

    // Pools
    //err = rico_serialize(&chunk->textures, file);
    //if (err) return err;
    err = rico_serialize(&chunk->materials, file);
    if (err) return err;
    //err = rico_serialize(&chunk->meshes, file);
    //if (err) return err;
    err = rico_serialize(&chunk->objects, file);
    if (err) return err;

    #ifdef RICO_DEBUG_CHUNK
        printf("[chnk][save] name=%s filename=%s\n", chunk->uid.name,
               file->filename);
    #endif

    return err;
}

int chunk_deserialize_0(void *_handle, const struct rico_file *file)
{
    enum rico_error err;
    struct rico_chunk *_chunk = _handle;

    fread(&_chunk->version,    sizeof(_chunk->version),    1, file->fs);
    fread(&_chunk->tex_count,  sizeof(_chunk->tex_count),  1, file->fs);
    fread(&_chunk->mat_count,  sizeof(_chunk->mat_count),  1, file->fs);
    fread(&_chunk->mesh_count, sizeof(_chunk->mesh_count), 1, file->fs);
    fread(&_chunk->obj_count,  sizeof(_chunk->obj_count),  1, file->fs);

    // Pools
    //err = rico_deserialize(&_chunk->textures, file);
    //if (err) return err;

    err = rico_deserialize(&_chunk->materials, file);
    if (err) return err;
    material_pool_set_unsafe(&_chunk->materials);

    //err = rico_deserialize(&_chunk->meshes, file);
    //if (err) return err;

    err = rico_deserialize(&_chunk->objects, file);
    if (err) return err;
    object_pool_set_unsafe(&_chunk->objects);

#ifdef RICO_DEBUG_CHUNK
    printf("[chnk][load] name=%s filename=%s\n", _chunk->uid.name,
           file->filename);
#endif

    return err;
}

#ifdef RICO_DEBUG_CHUNK
static void chunk_print(struct rico_chunk *chunk)
{
    printf("[chnk][show] name=%s tex=%d mat=%d mesh=%d obj=%d\n",
           chunk->uid.name, chunk->tex_count, chunk->mat_count,
           chunk->mesh_count, chunk->obj_count);
}
#endif