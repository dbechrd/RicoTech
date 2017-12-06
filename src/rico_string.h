#ifndef RICO_STRING_H
#define RICO_STRING_H

struct rico_string {
    struct hnd hnd;

    // TODO: Good grief.. what even is this?
    struct rico_object *object;
    u32 lifespan;
};
extern const u32 RICO_STRING_SIZE;

// Cleanup: Shouldn't needs these anymore
#if 0
#define RICO_STRING_SLOTS(f)    \
    f(STR_SLOT_DEBUG)           \
    f(STR_SLOT_SELECTED_OBJ)    \
    f(STR_SLOT_EDIT_INFO)       \
    f(STR_SLOT_FPS)             \
    f(STR_SLOT_MENU_QUIT)       \
    f(STR_SLOT_COUNT)           \
    f(STR_SLOT_DYNAMIC)
#endif

int string_request_by_name(struct rico_string *_str, const char *name);
int string_init(const char *name, float x, float y, struct col4 color,
                u32 lifespan, struct rico_font *font, const char *text);
int string_free(struct rico_string *str);
int string_update(r64 dt);

#endif // RICO_STRING_H
