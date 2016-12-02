#ifndef UID_H
#define UID_H

#include "const.h"
#include "rico_string.h"
#include <stdio.h>

#define RICO_UID_TYPES(f) \
    f(RICO_UID_NULL)    \
    f(RICO_UID_OBJECT)  \
    f(RICO_UID_TEXTURE) \
    f(RICO_UID_MESH)    \
    f(RICO_UID_CHUNK)   \
    f(RICO_UID_POOL)    \
    f(RICO_UID_BBOX)    \
    f(RICO_UID_COUNT)

enum rico_uid_type {
    RICO_UID_TYPES(GEN_LIST)
};
extern const char *rico_uid_type_string[];

struct rico_pool;

// TODO: Replace uint32 handle with null pointer directly to pool object
typedef int (*Serializer)(const void *handle, FILE *fs);
typedef int (*Deserializer)(void *_handle, FILE *fs);

extern Serializer RicoSerializers[RICO_UID_COUNT];
extern Deserializer RicoDeserializers[RICO_UID_COUNT];

extern Serializer rico_serializer;
extern Deserializer rico_deserializer;

struct rico_uid {
    enum rico_uid_type type;
    uint32 uid;
    char name[30]; // TODO: Should I dynamically allocating name strings?
};

extern struct rico_uid UID_NULL;

void uid_init(struct rico_uid *_uid, enum rico_uid_type type, const char *name);

#endif // UID_H
