#ifndef RICO_CEREAL_H
#define RICO_CEREAL_H

#include <stdio.h>

typedef int (*Serializer)(const void *handle, FILE *fs);
typedef int (*Deserializer)(void *_handle, FILE *fs);

extern Serializer RicoSerializers[];
extern Deserializer RicoDeserializers[];

extern Serializer rico_serialize;
extern Deserializer rico_deserialize;

#endif // RICO_CEREAL_H
