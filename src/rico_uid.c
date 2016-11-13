#include "rico_uid.h"
#include <string.h>
#include <stdio.h>

struct rico_uid UID_NULL = { 0, "NULL" };
static uint32 next_uid = 1;

void uid_init(const char *name, struct rico_uid *_uid)
{
    strncpy(_uid->name, name, sizeof(_uid->name));
    _uid->uid = next_uid++;

#ifdef RICO_DEBUG_INFO
    printf("[UID] %d %s\n", _uid->uid, _uid->name);
#endif
}