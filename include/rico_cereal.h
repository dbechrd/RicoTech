#ifndef RICO_CEREAL_H
#define RICO_CEREAL_H

#include "const.h"
#include "rico_file.h"

typedef int (*Serializer)(const void *handle, const struct rico_file *file);
typedef int (*Deserializer)(void *_handle, const struct rico_file *file);

struct rico_cereal {
    Serializer save[RICO_FILE_VERSION_COUNT];
    Deserializer load[RICO_FILE_VERSION_COUNT];
};

extern struct rico_cereal RicoCereal[];

int rico_serialize(const void *handle, const struct rico_file *file);
int rico_deserialize(void *_handle, const struct rico_file *file);

//extern Serializer rico_serialize;
//extern Deserializer rico_deserialize;

#endif // RICO_CEREAL_H
