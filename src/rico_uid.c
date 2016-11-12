#include "rico_uid.h"
#include <string.h>

struct rico_uid UID_NULL = { 0, "NULL" };
static uint32 next_uid = 1;

void uid_init(const char *name, struct rico_uid *_uid)
{
    _uid->name = name;
    _uid->uid = next_uid++;
}