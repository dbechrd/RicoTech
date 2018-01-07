#ifndef RICO_STRING_H
#define RICO_STRING_H

#define RICO_STRING_SLOTS(f) \
    f(STR_SLOT_DEBUG)        \
    f(STR_SLOT_SELECTED_OBJ) \
    f(STR_SLOT_STATE)        \
    f(STR_SLOT_FPS)          \
    f(STR_SLOT_MENU_QUIT)    \
    f(STR_SLOT_DELTA)        \
    f(STR_SLOT_DYNAMIC)
enum rico_string_slot
{
    RICO_STRING_SLOTS(GEN_LIST)
};
extern const char *rico_string_slot_string[];

struct rico_string
{
    u32 id;
    enum rico_string_slot slot;
    u32 object_id;
    u32 lifespan;
};

void string_delete(struct pack *pack, struct rico_string *str);
bool string_free_slot(enum rico_string_slot slot);
void string_update();

#endif