#ifndef RICO_INTERNAL_STRING_H
#define RICO_INTERNAL_STRING_H

#include "RICO/rico_string.h"

struct RICO_string
{
    struct uid uid;
    enum RICO_string_slot slot;
    pkid object_id;
    u32 lifespan;
};

static void string_delete(struct RICO_string *str);
static bool string_free_slot(enum RICO_string_slot slot);
static void string_update();

#endif