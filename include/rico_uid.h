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

struct rico_uid {
    enum rico_uid_type type;
    uint32 version;
    uint32 uid;
    char name[30];
};

extern struct rico_uid UID_NULL;

void uid_init(struct rico_uid *_uid, enum rico_uid_type type, uint32 version,
              const char *name);
int uid_serialize(const void *handle, FILE *fs);
int uid_deserialize(void *_handle, FILE *fs);

#endif // UID_H
