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
enum rico_string_slot {
    RICO_STRING_SLOTS(GEN_LIST)
};
extern const char *rico_string_slot_string[];

struct rico_string {
    struct hnd hnd;
    enum rico_string_slot slot;
    struct pool_id object_id;
    u32 lifespan;
};

int string_init(struct rico_string *str, const char *name,
                enum rico_string_slot slot, float x, float y, struct col4 color,
                u32 lifespan, struct rico_font *font, const char *text);
int string_free(struct rico_string *str);
int string_free_slot(enum rico_string_slot slot);
int string_update(r64 dt);

#endif // RICO_STRING_H
