#ifndef RICO_STRING_H
#define RICO_STRING_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_string {
    struct rico_uid uid;
    struct hnd object;
    u32 lifespan;
};
extern const u32 RICO_STRING_SIZE;

#define RICO_STRING_SLOTS(f)    \
    f(STR_SLOT_NULL)            \
    f(STR_SLOT_DEBUG)           \
    f(STR_SLOT_SELECTED_OBJ)    \
    f(STR_SLOT_EDIT_INFO)       \
    f(STR_SLOT_FPS)             \
    f(STR_SLOT_MENU_QUIT)       \
    f(STR_SLOT_COUNT)           \
    f(STR_SLOT_DYNAMIC)

enum rico_string_slot {
    RICO_STRING_SLOTS(GEN_LIST)
};
extern const char *rico_string_slot_string[];

int string_request_by_name(struct hnd *_handle, enum rico_persist persist,
                           const char *name);
int string_init(enum rico_persist persist, const char *name,
                enum rico_string_slot slot, float x, float y, struct col4 color,
                u32 lifespan, struct hnd font, const char *text);
int string_free(struct hnd handle);
int string_update(r64 dt);

#endif // RICO_STRING_H