#include "rico_chunk.h"
#include "const.h"

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

static uint32 parse_mesh(const char *mesh)
{
    UNUSED(mesh);
    return 0;
}


struct rico_chunk *chunk_load(const char *filename)
{
    UNUSED(filename);
    //header
    //[mesh count] [texture count] [object count]
    //mesh [handle] [name_len] [name] [program_id] [vertex count] [vertices]
    //[element count] [elements]
    //mesh ...
    //texture [handle] [name_len] [name] 

    return 0;
}