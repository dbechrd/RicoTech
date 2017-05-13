#include "rico_chunk.h"
#include "const.h"
#include "rico_cereal.h"
#include "rico_object.h"
#include "rico_material.h"
#include "rico_font.h"
#include <stdio.h>
#include <stdlib.h>

static void chunk_print(struct rico_chunk *chunk);

static u32 parse_mesh(const char *mesh)
{
    UNUSED(mesh);
    return 0;
}

int chunk_init(const char *name, u32 version, u32 strings, u32 fonts,
               u32 textures, u32 materials, u32 meshes, u32 objects,
               struct rico_chunk *_chunk)
{
    u32 size1 = sizeof(struct rico_chunk);
    u32 size2 = POOL_SIZE(strings,   RICO_STRING_SIZE);
    u32 size3 = POOL_SIZE(fonts,     RICO_FONT_SIZE);
    u32 size4 = POOL_SIZE(textures,  RICO_TEXTURE_SIZE);
    u32 size5 = POOL_SIZE(materials, RICO_MATERIAL_SIZE);
    u32 size6 = POOL_SIZE(meshes,    RICO_MESH_SIZE);
    u32 size7 = POOL_SIZE(objects,   RICO_OBJECT_SIZE);
    u32 total_size = size1 + size2 + size3 + size4 + size5 + size6 + size7;

    // TODO: Clean up how getting these addresses works; possibly refactor
    //       the alloc farther up than this.
    void *mem_block = calloc(1, total_size);
    if (!mem_block)
    {
        return RICO_ERROR(ERR_BAD_ALLOC, "Failed to alloc memory for chunk %s",
                          name);
    }

    struct rico_chunk *chunk = mem_block;
    uid_init(&chunk->uid, RICO_UID_CHUNK, name, true);
    chunk->version = version;
    chunk->size = total_size;

    // Calculate pool offsets
    chunk->strings   = (struct rico_pool *)((u8 *)mem_block + size1);
    chunk->fonts     = (struct rico_pool *)((u8 *)mem_block + size2);
    chunk->textures  = (struct rico_pool *)((u8 *)mem_block + size3);
    chunk->materials = (struct rico_pool *)((u8 *)mem_block + size4);
    chunk->meshes    = (struct rico_pool *)((u8 *)mem_block + size5);
    chunk->objects   = (struct rico_pool *)((u8 *)mem_block + size6);

    pool_init(chunk->strings,   "Strings",   strings,   RICO_STRING_SIZE,   STR_SLOT_DYNAMIC);
    pool_init(chunk->fonts,     "Fonts",     fonts,     RICO_FONT_SIZE,     0);
    pool_init(chunk->textures,  "Textures",  textures,  RICO_TEXTURE_SIZE,  0);
    pool_init(chunk->materials, "Materials", materials, RICO_MATERIAL_SIZE, 0);
    pool_init(chunk->meshes,    "Meshes",    meshes,    RICO_MESH_SIZE,     0);
    pool_init(chunk->objects,   "Objects",   objects,   RICO_OBJECT_SIZE,   0);

#ifdef RICO_DEBUG_CHUNK
    printf("[chnk][init] name=%s\n", chunk->uid.name);
    chunk_print(chunk);
#endif

    _chunk = chunk;
    return SUCCESS;
}

//int chunk_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(chunk_serialize_0)
{
    enum rico_error err = SUCCESS;
    const struct rico_chunk *chunk = handle;

    // TODO: Check fwrite success
    // Write chunk to file
    fwrite(&chunk, chunk->size, 1, file->fs);

    #ifdef RICO_DEBUG_CHUNK
        printf("[chnk][save] name=%s filename=%s size=%d\n", chunk->uid.name,
               file->filename, chunk->size);
    #endif

    return err;
}

//int chunk_deserialize_0(void *_handle, const struct rico_file *file)
DESERIAL(chunk_deserialize_0)
{
    enum rico_error err = SUCCESS;
    struct rico_chunk *_chunk = _handle;

    // TODO: Read chunk size from file so we know how much to allocate, then
    //       initialize the chunk properly before calling fread().
    RICO_ASSERT(0);

    // TODO: Check fread success
    // Read chunk from file
    fread(&_chunk, _chunk->size, 1, file->fs);

#ifdef RICO_DEBUG_CHUNK
    printf("[chnk][load] name=%s filename=%s size=%d\n", _chunk->uid.name,
           file->filename, _chunk->size);
#endif

    return err;
}

#ifdef RICO_DEBUG_CHUNK
static void chunk_print(struct rico_chunk *chunk)
{
    // TODO: Print more information about the pool sizes
    printf("[chnk][show] name=%s size=%d\n", chunk->uid.name, chunk->size);
}
#endif