#ifndef RICO_UI_H
#define RICO_UI_H

////////////////////////////////////////////////////////////////////////////////
// TODO: Move this to its own header if it works

struct RICO_sprite
{
    struct RICO_spritesheet *sheet;
    struct vec2 uvs[2];
};

struct RICO_spritesheet
{
    pkid tex_id;
    u32 sprite_count;
    struct RICO_sprite *sprites;
};

////////////////////////////////////////////////////////////////////////////////

struct ui_string
{
    const char *text;

    // TODO: Generate these automatically if null, cache for next frame, only
    //       update if text is dirty.
    pkid mesh_id;
    pkid material_id;
};

#define RICO_UI_ELEMENT_TYPES(f) \
    f(RICO_UI_ELEMENT_HUD)    \
    f(RICO_UI_ELEMENT_BUTTON) \
    f(RICO_UI_ELEMENT_LABEL)

enum RICO_ui_element_type
{
    RICO_UI_ELEMENT_TYPES(GEN_LIST)
    RICO_UI_ELEMENT_COUNT
};
extern const char *RICO_ui_element_type_string[];

enum RICO_ui_event_type
{
    RICO_UI_EVENT_HOVER,
    RICO_UI_EVENT_LMB_CLICK,
    RICO_UI_EVENT_LMB_DOWN,
    RICO_UI_EVENT_LMB_UP,
    RICO_UI_EVENT_MMB_CLICK,
    RICO_UI_EVENT_MMB_DOWN,
    RICO_UI_EVENT_MMB_UP,
    RICO_UI_EVENT_RMB_CLICK,
    RICO_UI_EVENT_RMB_DOWN,
    RICO_UI_EVENT_RMB_UP
};

struct RICO_ui_event
{
    struct RICO_ui_element *element;
    enum RICO_ui_event_type event_type;
};

typedef void (*RICO_ui_event_handler)(const struct RICO_ui_event *e);

struct RICO_ui_element
{
    enum RICO_ui_element_type type;
    
    struct vec2i min_size;
    struct rect size;    // includes margin
    struct rect margin;
    struct rect bounds;  // excludes margin
    struct rect padding;
    void *metadata;

    //struct RICO_ui_element *parent;
    //struct RICO_ui_element *prev;
    struct RICO_ui_element *next;
    struct RICO_ui_element *first_child;
    struct RICO_ui_element *last_child;

    RICO_ui_event_handler event;
};

struct RICO_ui_hud
{
    struct RICO_ui_element element;
    struct vec4 color;
};

enum RICO_ui_button_state
{
    RICO_UI_BUTTON_DEFAULT,
    RICO_UI_BUTTON_HOVERED,
    RICO_UI_BUTTON_PRESSED,
    
    RICO_UI_BUTTON_COUNT
};

struct RICO_ui_button
{
    struct RICO_ui_element element;
    struct RICO_sprite *sprite;
    enum RICO_ui_button_state state;
    struct vec4 color[RICO_UI_BUTTON_COUNT];
};

struct RICO_ui_label
{
    struct RICO_ui_element element;
    struct vec4 color;
    struct ui_string *string;
};

extern struct RICO_ui_hud *RICO_ui_hud();
extern struct RICO_ui_label *RICO_ui_label(struct RICO_ui_element *parent);
extern bool RICO_ui_layout(struct RICO_ui_element *element, s32 x, s32 y,
                           s32 max_w, s32 max_h);
extern void RICO_ui_draw(struct RICO_ui_element *element, s32 x, s32 y);

#endif