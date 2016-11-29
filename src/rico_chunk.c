#include "rico_chunk.h"
#include "const.h"
#include "util.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "rico_object.h"
#include "bbox.h"

// TODO: Create resource loaders to handle:
//       fonts, shaders, textures, meshes, etc.

// meshes.bin
//

// textures.bin
//

// objects.bin
//  uid     (store or generate??)
//  name    [hash]
//  pos
//  rot
//  scale
//  mesh    [id/uid/handle]
//  texture [id/uid/handle]


//header
//[texture count] [mesh count] [object count]
//texture [handle] [name_len] [name]
//mesh [handle] [name_len] [name] [program_id] [vertex count] [vertices]
//[element count] [elements]

#define SIGNATURE_SIZE 4
static const char SIGNATURE[SIGNATURE_SIZE] = { 'R', '1', 'C', '0' };

static void chunk_print(struct rico_chunk *chunk);

static uint32 parse_mesh(const char *mesh)
{
    UNUSED(mesh);
    return 0;
}

int chunk_init(const char *name, uint32 tex_count, uint32 mesh_count,
               uint32 obj_count, struct rico_pool *textures,
               struct rico_pool *meshes, struct rico_pool *objects,
               struct rico_chunk *_chunk)
{
    UNUSED(tex_count);
    UNUSED(mesh_count);
    UNUSED(textures);
    UNUSED(meshes);

    struct rico_chunk chunk;
    uid_init(&chunk.uid, RICO_UID_CHUNK, name);
    chunk.tex_count = 0; //tex_count;
    chunk.mesh_count = 0; //mesh_count;
    chunk.obj_count = obj_count;
    chunk.textures = (struct rico_pool) { 0 }; //*textures;
    chunk.meshes = (struct rico_pool) { 0 }; //*meshes;
    chunk.objects = *objects;

#ifdef RICO_DEBUG_CHUNK
    printf("[Chunk %d][%s] Initialized\n", chunk.uid.uid, chunk.uid.name);
    chunk_print(&chunk);
#endif

    *_chunk = chunk;
    return SUCCESS;
}

int chunk_save(const char *filename, const struct rico_chunk *chunk)
{
    FILE *fs = fopen(filename, "wb");
    if (!fs) {
        fprintf(stderr, "Unable to open %s for writing\n", filename);
        return ERR_FILE_WRITE;
    }

    // TODO: Separate dynamic data from static data. Textures, meshes, and
    //       static objects should be in their own file. Dynamic objects
    //       should be loaded as part of the save file. Save file can also
    //       override static objects' positions, states, etc.

    //--------------------------------------------------------------------------
    // fwrite(_SrcBuf, _ElementSize, _Count, _Stream)
    //--------------------------------------------------------------------------

    // File signature
    fwrite(SIGNATURE, sizeof(SIGNATURE), 1, fs);

    // Chunk header
    fwrite(&chunk->uid,        sizeof(chunk->uid),        1, fs);
    fwrite(&chunk->tex_count,  sizeof(chunk->tex_count),  1, fs);
    fwrite(&chunk->mesh_count, sizeof(chunk->mesh_count), 1, fs);
    fwrite(&chunk->obj_count,  sizeof(chunk->obj_count),  1, fs);

    // Pools
    pool_serialize(&chunk->textures, fs);
    pool_serialize(&chunk->meshes, fs);
    pool_serialize(&chunk->objects, fs);

    fclose(fs);

#ifdef RICO_DEBUG_CHUNK
    printf("[Chunk %d][%s] Chunk saved to %s\n", chunk->uid.uid,
           chunk->uid.name, filename);
#endif
    return SUCCESS;
}

//header
//[mesh count] [texture count] [object count]
//mesh [handle] [name_len] [name] [program_id] [vertex count] [vertices]
//[element count] [elements]
//mesh ...
//texture [handle] [name_len] [name]

int chunk_load(const char *filename, struct rico_chunk *_chunk)
{
    FILE *fs = fopen(filename, "rb");
    if (!fs) {
        fprintf(stderr, "Unable to open %s for reading\n", filename);
        return ERR_FILE_READ;
    }

    // TODO: Wrap fread() calls in something that checks if fread() return
    //       value (bytes read) is less than requested bytes. If so, check
    //       for EOF with feof() or error with ferror().

    //--------------------------------------------------------------------------
    // fread(_DstBuf, _ElementSize, _Count, _Stream)
    //--------------------------------------------------------------------------
    char SIGNATURE_BUFFER[SIGNATURE_SIZE];
    fread(&SIGNATURE_BUFFER, SIGNATURE_SIZE, 1, fs);
    if (strncmp(SIGNATURE_BUFFER, SIGNATURE, SIGNATURE_SIZE))
    {
       fprintf(stderr, "Invalid chunk signature\n");
       return ERR_FILE_READ;
    }

    // Chunk header
    fread(&_chunk->uid,        sizeof(_chunk->uid),        1, fs);
    fread(&_chunk->tex_count,  sizeof(_chunk->tex_count),  1, fs);
    fread(&_chunk->mesh_count, sizeof(_chunk->mesh_count), 1, fs);
    fread(&_chunk->obj_count,  sizeof(_chunk->obj_count),  1, fs);

    // Pools
    pool_deserialize(&_chunk->textures, fs);
    pool_deserialize(&_chunk->meshes, fs);
    pool_deserialize(&_chunk->objects, fs);

    fclose(fs);

#ifdef RICO_DEBUG_CHUNK
    printf("[Chunk %d][%s] Chunk loaded from %s\n", _chunk->uid.uid,
           _chunk->uid.name, filename);
#endif
    return SUCCESS;
}

#ifdef RICO_DEBUG_CHUNK
static void chunk_print(struct rico_chunk *chunk)
{
    printf("[Chunk %d][%s] Tex: %d Mesh: %d Obj: %d\n", chunk->uid.uid,
           chunk->uid.name, chunk->tex_count, chunk->mesh_count,
           chunk->obj_count);
}
#endif