#ifndef UID_H
#define UID_H

#include "const.h"

struct rico_uid {
    uint32 uid;
    char name[50];
};

extern struct rico_uid UID_NULL;

void uid_init(const char *name, struct rico_uid *result);

#endif // UID_H
