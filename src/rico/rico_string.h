#ifndef RICO_STRING_H
#define RICO_STRING_H

#define RICO_STRING_SLOTS(f) \
    f(STR_SLOT_SELECTED_OBJ) \
    f(STR_SLOT_STATE)        \
    f(STR_SLOT_FPS)          \
    f(STR_SLOT_MENU_QUIT)    \
    f(STR_SLOT_DELTA)        \
    f(STR_SLOT_WIDGET)       \
    f(STR_SLOT_DEBUG_CAMERA)

enum RICO_string_slot
{
    RICO_STRING_SLOTS(GEN_LIST)
    STR_SLOT_COUNT
};
extern const char *RICO_string_slot_string[];

#endif