#ifndef RICO_CEREAL_H
#define RICO_CEREAL_H

#include "const.h"
#include "rico_file.h"

#define SERIAL(name) int name(const void *handle, const struct rico_file *file)
#define DESERIAL(name) int name(void *_handle, const struct rico_file *file)
typedef SERIAL(serializer);
typedef DESERIAL(deserializer);

struct rico_cereal {
    serializer *save[RICO_FILE_VERSION_COUNT];
    deserializer *load[RICO_FILE_VERSION_COUNT];
};

extern struct rico_cereal rico_cereals[];

SERIAL(rico_serialize);
DESERIAL(rico_deserialize);

//extern Serializer rico_serialize;
//extern Deserializer rico_deserialize;

#endif // RICO_CEREAL_H
