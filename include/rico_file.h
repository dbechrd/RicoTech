#ifndef RICO_FILE_H
#define RICO_FILE_H

//#include "const.h"
//#include <stdio.h>

#define RICO_FILE_VERSION_CURRENT 1
#define RICO_FILE_VERSION_MINIMUM_SUPPORTED 1
#define RICO_FILE_VERSION_MAXIMUM_SUPPORTED RICO_FILE_VERSION_CURRENT
#define RICO_FILE_VERSION_COUNT 1 + (RICO_FILE_VERSION_MAXIMUM_SUPPORTED -\
                                     RICO_FILE_VERSION_MINIMUM_SUPPORTED)

struct rico_file {
    FILE *fs;
    u32 version;
    u32 cereal_index;
    const char *filename;
};

int rico_file_open_write(struct rico_file *_handle, const char *filename,
                         u32 version);
int rico_file_open_read(struct rico_file *_handle, const char *filename);
void rico_file_close(struct rico_file *handle);

#endif // RICO_FILE_H