#ifndef UID_H
#define UID_H

#include "const.h"
#include "rico_string.h"

struct rico_uid {
    uint32 uid;
    const char *name;
};

extern struct rico_uid UID_NULL;

void uid_init(const char *name, struct rico_uid *_uid);

#endif // UID_H
