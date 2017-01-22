#ifndef RICO_CEREAL_H
#define RICO_CEREAL_H

#include "const.h"
#include "rico_file.h"

typedef int (*serializer)(const void *handle, const struct rico_file *file);
typedef int (*deserializer)(void *_handle, const struct rico_file *file);

struct rico_cereal {
    serializer save[RICO_FILE_VERSION_COUNT];
    deserializer load[RICO_FILE_VERSION_COUNT];
};

extern struct rico_cereal rico_cereals[];

int rico_serialize(const void *handle, const struct rico_file *file);
int rico_deserialize(void *_handle, const struct rico_file *file);

//extern Serializer rico_serialize;
//extern Deserializer rico_deserialize;

#endif // RICO_CEREAL_H
