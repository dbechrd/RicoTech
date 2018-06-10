#ifndef RICO_UI_H
#define RICO_UI_H

#define RICO_UI_ELEMENT_TYPES(f) \
    f(RICO_UI_ELEMENT_HUD)   \
    f(RICO_UI_ELEMENT_ROW)   \
    f(RICO_UI_ELEMENT_LABEL)

enum RICO_ui_element_type
{
    RICO_UI_ELEMENT_TYPES(GEN_LIST)
    RICO_UI_ELEMENT_COUNT
};
extern const char *RICO_ui_element_type_string[];

struct RICO_ui_element
{
    enum RICO_ui_element_type type;
    
    struct vec2i min_size;
    struct rect size;    // includes margin
    struct rect margin;
    struct rect bounds;  // excludes margin
    struct rect padding;

    struct RICO_ui_element *parent;
    struct RICO_ui_element *prev;
    struct RICO_ui_element *next;
    struct RICO_ui_element *first_child;
    struct RICO_ui_element *last_child;
};

struct RICO_ui_hud
{
    struct RICO_ui_element element;
};

struct RICO_ui_row
{
    struct RICO_ui_element element;
};

struct RICO_ui_label
{
    struct RICO_ui_element element;
    const char *text;

    // TODO: Generate these automatically if null, cache for next frame, only
    //       update if text is dirty.
    pkid mesh_id;
    pkid material_id;
};

extern struct RICO_ui_hud *RICO_ui_push_hud(const struct vec2i *min_size,
                                            const struct rect *margin,
                                            const struct rect *padding);
extern struct RICO_ui_row *RICO_ui_push_row(struct RICO_ui_element *parent,
                                            const struct vec2i *min_size,
                                            const struct rect *margin,
                                            const struct rect *padding);
extern struct RICO_ui_label *RICO_ui_push_label(struct RICO_ui_element *parent,
                                                const struct vec2i *min_size,
                                                const struct rect *margin,
                                                const struct rect *padding);
extern bool RICO_ui_draw(struct RICO_ui_element *element, u32 x, u32 y,
                         u32 max_w, u32 max_h);

#endif