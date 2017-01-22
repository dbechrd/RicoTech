#ifndef RICO_STRING_H
#define RICO_STRING_H

#include "const.h"
struct col4;
struct rico_font;

#define RICO_STRING_SLOTS(f)    \
    f(STR_SLOT_NULL)            \
    f(STR_SLOT_SELECTED_OBJ)    \
    f(STR_SLOT_EDIT_INFO)       \
    f(STR_SLOT_FPS)             \
    f(STR_SLOT_COUNT)           \
    f(STR_SLOT_DYNAMIC)

enum rico_string_slot {
    RICO_STRING_SLOTS(GEN_LIST)
};
extern const char *rico_string_slot_string[];

int rico_string_init(u32 pool_size);
int string_init(const char *name, enum rico_string_slot slot, u32 x, u32 y,
                struct col4 color, u32 lifespan, u32 font, const char *text);
int string_free(u32 handle);
int string_update(u32 dt);

#endif // RICO_STRING_H