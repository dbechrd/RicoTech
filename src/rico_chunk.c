#include "rico_chunk.h"
#include "const.h"
#include "util.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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

static const char SIGNATURE[2] = { 0x52, 0x54 };

static uint32 parse_mesh(const char *mesh)
{
    UNUSED(mesh);
    return 0;
}

int chunk_save(const char *filename, const struct rico_chunk *data)
{
    //header
    //[mesh count] [texture count] [object count]
    //mesh [handle] [name_len] [name] [program_id] [vertex count] [vertices]
    //[element count] [elements]
    //mesh ...
    //texture [handle] [name_len] [name]


    FILE *fs = fopen(filename, "wb");
    if (!fs) {
        fprintf(stderr, "Unable to open %s for writing\n", filename);
        return ERR_FILE_LOAD;
    }

    //source buffer, size, count, file
    fwrite(SIGNATURE, 1, sizeof(SIGNATURE), fs);
    fwrite(data->data, 1, strlen(data->data), fs);
    fclose(fs);

    return SUCCESS;
}

int chunk_load(const char *filename, struct rico_chunk *_data)
{
    int length;
    char *buffer = file_contents(filename, &length);

    if (length >= 2 && strncmp(buffer, SIGNATURE, sizeof(SIGNATURE))) {
        fprintf(stderr, "Invalid chunk signature\n");
        return ERR_FILE_LOAD;
    }

    //header
    //[mesh count] [texture count] [object count]
    //mesh [handle] [name_len] [name] [program_id] [vertex count] [vertices]
    //[element count] [elements]
    //mesh ...
    //texture [handle] [name_len] [name]

    _data->data = buffer;
    return SUCCESS;
}