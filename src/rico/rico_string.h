#ifndef RICO_STRING_H
#define RICO_STRING_H

#define RICO_STRING_SLOTS(f) \
    f(STR_SLOT_SELECTED_OBJ, 0x1) \
    f(STR_SLOT_STATE,        0x2) \
    f(STR_SLOT_FPS,          0x3) \
    f(STR_SLOT_MENU_QUIT,    0x4) \
    f(STR_SLOT_DELTA,        0x5) \
    f(STR_SLOT_WIDGET,       0x6) \
    f(STR_SLOT_DEBUG_CAMERA, 0x7) \
    f(STR_SLOT_DYNAMIC,      0x8)

enum RICO_string_slot
{
    RICO_STRING_SLOTS(GEN_LIST_VALUES)
    STR_SLOT_COUNT
};
extern const char *RICO_string_slot_string[];

#endif