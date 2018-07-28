#ifndef RICO_INTERNAL_STRING_H
#define RICO_INTERNAL_STRING_H

#include "rico_string.h"

struct RICO_string
{
    struct uid uid;
    u32 slot;
    u32 lifespan;

    pkid mesh_id;
    pkid texture_id;
};

extern pkid global_string_slots[STR_SLOT_COUNT + 64];

static void string_delete(struct RICO_string *str);
static void string_free_slot(enum RICO_string_slot slot);
static void string_update();
static void string_render(struct RICO_string *str, GLint model_location);
static void string_render_all(GLint model_location);

#endif