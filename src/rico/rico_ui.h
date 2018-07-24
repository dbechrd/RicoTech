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

#define MAX_TOOLTIPS 32
struct ui_tooltip
{
    bool enabled;
    struct vec4 color;
    struct RICO_heiro_string *string;
};

#define RICO_UI_ELEMENT_TYPES(f) \
    f(RICO_UI_ELEMENT_HUD)       \
    f(RICO_UI_ELEMENT_BREAK)     \
    f(RICO_UI_ELEMENT_STRING)    \
    f(RICO_UI_ELEMENT_BUTTON)    \
    f(RICO_UI_ELEMENT_LABEL)     \
    f(RICO_UI_ELEMENT_PROGRESS)

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

struct RICO_ui_head
{
    enum RICO_ui_element_type type;
    struct RICO_ui_head *next;

    //struct RICO_ui_element *parent;
    //struct RICO_ui_element *prev;
};

struct RICO_ui_element
{
    struct RICO_ui_head head;
    struct vec2i min_size;
    struct rect size;    // includes margin
    struct rect margin;
    struct rect bounds;  // excludes margin
    struct rect padding;
    void *metadata;

    RICO_ui_event_handler event;
};

/*
// Cleanup: HUD is the only container, right?
struct RICO_ui_container
{
    struct RICO_ui_element element;
    struct RICO_ui_head *first_child;
    struct RICO_ui_head *last_child;
};
*/

struct RICO_ui_hud
{
    struct RICO_ui_element element;
    struct RICO_ui_head *first_child;
    struct RICO_ui_head *last_child;
    struct vec4 color;
};

enum RICO_ui_state
{
    RICO_UI_DEFAULT,
    RICO_UI_HOVERED,
    RICO_UI_PRESSED,
    RICO_UI_COUNT
};

struct RICO_ui_button
{
    struct RICO_ui_element element;
    enum RICO_ui_state state;
    struct vec4 color[RICO_UI_COUNT];
    struct RICO_sprite *sprite;
    struct ui_tooltip *tooltip;
};

struct RICO_ui_label
{
    struct RICO_ui_element element;
    struct vec4 color;
    struct RICO_heiro_string *heiro;
};

struct RICO_ui_progress
{
    struct RICO_ui_element element;
    enum RICO_ui_state state;
    struct vec4 color;
    float percent;
    struct RICO_heiro_string *heiro;
    struct ui_tooltip *tooltip;
};

extern struct RICO_ui_hud *RICO_ui_hud();
extern struct RICO_ui_head *RICO_ui_line_break(struct RICO_ui_hud *parent);
extern struct RICO_ui_button *RICO_ui_button(struct RICO_ui_hud *parent);
extern struct RICO_ui_label *RICO_ui_label(struct RICO_ui_hud *parent);
extern struct RICO_ui_progress *RICO_ui_progress(struct RICO_ui_hud *parent);
extern bool RICO_ui_layout(struct RICO_ui_element *element, s32 x, s32 y,
                           s32 max_w, s32 max_h);
extern void RICO_ui_draw(struct RICO_ui_element *element, s32 x, s32 y);

#endif