#ifndef RICO_INTERNAL_STRING_H
#define RICO_INTERNAL_STRING_H

#include "RICO/rico_string.h"

struct rico_string
{
    struct uid uid;
    enum rico_string_slot slot;
    pkid object_id;
    u32 lifespan;
};

void string_delete(struct rico_string *str);
bool string_free_slot(enum rico_string_slot slot);
void string_update();

#endif
