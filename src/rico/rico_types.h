#ifndef RICO_HND_H
#define RICO_HND_H

////////////////////////////////////////////////////////////////////////////////

//TODO: Probably should prefix these? Possibly move to const.h?
static s32 SCREEN_WIDTH = 1600;
static s32 SCREEN_HEIGHT = 900;
#define SCREEN_ASPECT (float)SCREEN_WIDTH / SCREEN_HEIGHT

#define X_TO_NDC(x) ((float)(x) / (SCREEN_WIDTH / 2.0f) - 1.0f)
#define Y_TO_NDC(y) (-(float)(y) / (SCREEN_HEIGHT / 2.0f) + 1.0f)

// NOTE: Pixel origin is top-left of screen
// NOTE: NDC origin is center of screen
// e.g. [0, 0]     -> [-1.0f, 1.0f]
// e.g. [800, 450] -> [0.0f, 0.0f]

// Calculate relative x/y in pixels, returns x/y in normalized device
// coordinates (negative values are relative to right and bottom edges of
// screen)
#define SCREEN_X(x) (x >= 0.0f ? X_TO_NDC(x) : X_TO_NDC(x + SCREEN_WIDTH))
#define SCREEN_Y(y) (y >= 0.0f ? Y_TO_NDC(y) : Y_TO_NDC(y + SCREEN_HEIGHT))

// Takes width/height in pixels, returns width/height in NDC
#define SCREEN_W(x) ((float)(x) / SCREEN_WIDTH * 2.0f)
#define SCREEN_H(y) (-(float)(y) / SCREEN_HEIGHT * 2.0f)

#define Z_NEAR 1.0f
#define Z_FAR 32.0f

////////////////////////////////////////////////////////////////////////////////

typedef u32 pkid;

#define RICO_HND_TYPES(f)	 \
    f(RICO_HND_NULL,	  0) \
    f(RICO_HND_OBJECT,    sizeof(struct RICO_object))   \
    f(RICO_HND_TEXTURE,   sizeof(struct RICO_texture))	\
    f(RICO_HND_MESH,      sizeof(struct RICO_mesh))		\
    f(RICO_HND_FONT,      sizeof(struct RICO_font))		\
    f(RICO_HND_STRING,    sizeof(struct RICO_string))	\
    f(RICO_HND_MATERIAL,  sizeof(struct RICO_material))	\
    f(RICO_HND_BBOX,      sizeof(struct RICO_bbox))

enum RICO_hnd_type
{
    RICO_HND_TYPES(GEN_LIST)
    RICO_HND_COUNT
};
extern const char *RICO_hnd_type_string[];
extern const u32 RICO_hnd_type_size[];

typedef u8 buf32[32];

struct uid
{
    pkid pkid;
    enum RICO_hnd_type type;
    buf32 name;
};

#define RICO_AUDIO_STATE(f) \
    f(RICO_AUDIO_UNKNOWN)   \
    f(RICO_AUDIO_STOPPED)   \
    f(RICO_AUDIO_PLAYING)   \
    f(RICO_AUDIO_PAUSED)    \
    f(RICO_AUDIO_COUNT)

enum RICO_audio_state
{
    RICO_AUDIO_STATE(GEN_LIST)
};
extern const char *RICO_audio_state_string[];

struct RICO_audio_source
{
    ALuint al_source_id;
    float pitch;
    float gain;
    bool loop;
};

struct RICO_audio_buffer
{
    ALuint al_buffer_id;
};

////////////////////////////////////////////////////////////////////////////////
// TODO: Move this to rico_primitives

// TODO: Replace bbox with aabb
// TODO: Implement reuse of data for bounding boxes.. no need to initialize
//       an entirely new vao/vbo for every bbox.
// TODO: Don't serialize vao/vbo!

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct RICO_bbox
{
    struct vec3 min;
    struct vec3 max;
};

struct RICO_aabb
{
    struct vec3 c;
    struct vec3 e;  // half-width extents
};

struct RICO_obb
{
    // PERF: Only store two of the axes and calculate third using cross product
    struct vec3 c;     // center
    struct vec3 u[3];  // normalized axes
    struct vec3 e;     // half-width extents
};

struct RICO_camera
{
    struct vec3 pos;
    struct vec3 vel;
    struct vec3 acc;

    float pitch;
    float yaw;
    float roll;

    //TODO: Implement better camera with position + lookat. Is that necessary?
    //      Maybe it's easy to derive lookat when I need it? Probably not..
    struct quat view;
    float fov_deg;
    GLenum fill_mode;
    bool locked;
    bool need_update;

    struct RICO_bbox RICO_bbox; //TODO: Replace with vec3 and do v3_render()
    struct mat4 view_matrix;
    struct mat4 persp_matrix;
    struct mat4 ortho_matrix;
    struct mat4 *proj_matrix;
};

struct RICO_heiro_string
{
    struct rect bounds;
    u32 length;
    u32 vertex_count;
    struct text_vertex *verts;
};

enum RICO_light_type
{
    LIGHT_AMBIENT       = 0,
    LIGHT_DIRECTIONAL   = 1,
    LIGHT_POINT         = 2,
    LIGHT_SPOT          = 3
};

struct RICO_light
{
    enum RICO_light_type type;
    bool on;
    struct vec3 col;
    struct vec3 pos;
    float intensity;

    union
    {
        struct
        {
            struct vec3 dir;
        } directional;

        struct
        {
            // Distance fall-off
            float kc;  // Constant
            float kl;  // Linear
            float kq;  // Quadratic
        } point;

        struct
        {
            struct vec3 dir;

            // Point / Spot lights
            // Distance fall-off
            float kc;  // Constant
            float kl;  // Linear
            float kq;  // Quadratic

            // Angle of inner (full intensity) and outer (fall-off to zero) cone
            float theta_inner;
            float theta_outer;
        } spot;
    };
};

/*
struct light_ambient
{
    struct vec3 color;
};

struct light_directional
{
    struct vec3 color;
    struct vec3 direction;
};

struct light_point
{
    struct vec3 color;
    struct vec3 position;
    float intensity;

    // Distance fall-off
    float kc;  // Constant
    float kl;  // Linear
    float kq;  // Quadratic
};

struct light_spot
{
    struct vec3 color;
    struct vec3 position;
    struct vec3 direction;

    // Angle of inner (full intensity) and outer (fall-off to zero) cone
    float theta_inner;
    float theta_outer;

    // Distance fall-off
    float kc;  // Constant
    float kl;  // Linear
    float kq;  // Quadratic
};
*/

struct RICO_mesh
{
    struct uid uid;
    u32 vertex_size;
    u32 vertex_count;
    u32 element_count;
    enum program_type prog_type;
    struct RICO_bbox bbox;

    // TODO: Remove these fields. Load data directly into VRAM then free buffer.
    u32 vertices_offset;
    u32 elements_offset;
};

struct RICO_keychord
{
    u16 keys[3];
    u8 repeat;
    u8 on_release;
};

#define CHORD3(k0, k1, k2) (struct RICO_keychord) {{ k0, k1, k2 }, 0, 0 }
#define CHORD2(k0, k1) CHORD3(k0, k1, 0)
#define CHORD1(k0)     CHORD3(k0, 0, 0)

#define CHORD_UP3(k0, k1, k2) (struct RICO_keychord) {{ k0, k1, k2 }, 0, 1 }
#define CHORD_UP2(k0, k1) CHORD_UP3(k0, k1, 0)
#define CHORD_UP1(k0)     CHORD_UP3(k0, 0, 0)

#define CHORD_REPEAT3(k0, k1, k2) (struct RICO_keychord) {{ k0, k1, k2 }, 1, 0 }
#define CHORD_REPEAT2(k0, k1) CHORD_REPEAT3(k0, k1, 0)
#define CHORD_REPEAT1(k0)     CHORD_REPEAT3(k0, 0, 0)

#define CHORD_UP_REPEAT3(k0, k1, k2) \
    (struct RICO_keychord) {{ k0, k1, k2 }, 1, 1 }
#define CHORD_UP_REPEAT2(k0, k1) CHORD_UP_REPEAT3(k0, k1, 0)
#define CHORD_UP_REPEAT1(k0)     CHORD_UP_REPEAT3(k0, 0, 0)

#define RICO_SCANCODE_ALT   ((u16)301)
#define RICO_SCANCODE_CTRL  ((u16)302)
#define RICO_SCANCODE_SHIFT ((u16)303)
#define RICO_SCANCODE_LMB   ((u16)304)
#define RICO_SCANCODE_MMB   ((u16)305)
#define RICO_SCANCODE_RMB   ((u16)306)

//#define CHORD_3(action, k0, k1, k2) action_chords[action] = CHORD3(k0, k1, k2)
//#define CHORD_2(action, k0, k1)     action_chords[action] = CHORD2(k0, k1)
//#define CHORD_1(action, k0)         action_chords[action] = CHORD1(k0)

#if 0
struct rico_keysequence
{
    struct rico_keychord chords[3];
};

// NOTE: Key, chord, sequence
struct rico_keyevent
{
    enum rico_keyevent_type
    {
        RICO_KEYEVENT_KEY,
        RICO_KEYEVENT_CHORD,
        RICO_KEYEVENT_SEQUENCE
    } type;
    union
    {
        rico_key key;
        struct rico_keychord chord;
        struct rico_keysequence sequence;
    };
};

#define EVENT (struct rico_keyevent)
#define EVENT_KEY(key) EVENT{ RICO_KEYEVENT_KEY, key }
#define EVENT_CHORD(k0, k1) EVENT{ RICO_KEYEVENT_CHORD, k0, k1 }
#endif

#define RICO_STATES(f)      \
    f(STATE_PLAY_EXPLORE)   \
    f(STATE_EDIT_TRANSLATE) \
    f(STATE_EDIT_ROTATE)    \
    f(STATE_EDIT_SCALE)     \
    f(STATE_EDIT_MATERIAL)  \
    f(STATE_EDIT_MESH)      \
    f(STATE_MENU_QUIT)      \
    f(STATE_TEXT_INPUT)     \
    f(STATE_ENGINE_SHUTDOWN)

enum rico_state
{
    RICO_STATES(GEN_LIST)
    STATE_COUNT
};
extern const char *rico_state_string[STATE_COUNT];

enum RICO_action
{
    ACTION_NULL,

    ACTION_ENGINE_DEBUG_LIGHTING_TOGGLE,
    ACTION_ENGINE_DEBUG_TRIGGER_BREAKPOINT,
    ACTION_ENGINE_DEBUG_FOV_INCREASE,
    ACTION_ENGINE_DEBUG_FOV_DECREASE,
    ACTION_ENGINE_FPS_TOGGLE,
    ACTION_ENGINE_MOUSE_LOCK_TOGGLE,
    ACTION_ENGINE_VSYNC_TOGGLE,
    ACTION_ENGINE_MUTE_TOGGLE,
    ACTION_ENGINE_SIM_PAUSE,
    ACTION_ENGINE_QUIT,

    ACTION_CAMERA_SLOW_TOGGLE,
    ACTION_CAMERA_RESET,
    ACTION_CAMERA_LOCK_TOGGLE,
    ACTION_CAMERA_WIREFRAME_TOGGLE,

    ACTION_MOVE_RIGHT,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_UP,
    ACTION_MOVE_DOWN,
    ACTION_MOVE_FORWARD,
    ACTION_MOVE_BACKWARD,
    ACTION_MOVE_SPRINT,

    ACTION_PLAY_EDITOR,
    ACTION_PLAY_INTERACT,

    ACTION_EDIT_QUIT,
    ACTION_EDIT_SAVE,
    ACTION_EDIT_CYCLE_PACK,
    ACTION_EDIT_CYCLE_BLOB_REVERSE,
    ACTION_EDIT_CYCLE_BLOB,
    ACTION_EDIT_MOUSE_PICK_START,
    ACTION_EDIT_MOUSE_PICK_MOVE,
    ACTION_EDIT_MOUSE_PICK_END,
    ACTION_EDIT_BBOX_RECALCULATE_ALL,
    ACTION_EDIT_CREATE_OBJECT,
    ACTION_EDIT_SELECTED_DUPLICATE,
    ACTION_EDIT_SELECTED_DELETE,
    ACTION_EDIT_MODE_PREVIOUS,
    ACTION_EDIT_MODE_NEXT,
    ACTION_EDIT_CAMERA_TOGGLE_PROJECTION,
    ACTION_EDIT_DEBUG_TEXT_INPUT,

    ACTION_EDIT_TRANSLATE_RESET,
    ACTION_EDIT_TRANSLATE_UP,
    ACTION_EDIT_TRANSLATE_DOWN,
    ACTION_EDIT_TRANSLATE_WEST,
    ACTION_EDIT_TRANSLATE_EAST,
    ACTION_EDIT_TRANSLATE_NORTH,
    ACTION_EDIT_TRANSLATE_SOUTH,
    ACTION_EDIT_TRANSLATE_DELTA_INCREASE,
    ACTION_EDIT_TRANSLATE_DELTA_DECREASE,

    ACTION_EDIT_ROTATE_RESET,
    ACTION_EDIT_ROTATE_X_POS,
    ACTION_EDIT_ROTATE_X_NEG,
    ACTION_EDIT_ROTATE_Y_POS,
    ACTION_EDIT_ROTATE_Y_NEG,
    ACTION_EDIT_ROTATE_Z_POS,
    ACTION_EDIT_ROTATE_Z_NEG,
    ACTION_EDIT_ROTATE_DELTA_INCREASE,
    ACTION_EDIT_ROTATE_DELTA_DECREASE,
    ACTION_EDIT_ROTATE_HEPTAMODE_TOGGLE,

    ACTION_EDIT_SCALE_RESET,
    ACTION_EDIT_SCALE_UP,
    ACTION_EDIT_SCALE_DOWN,
    ACTION_EDIT_SCALE_WEST,
    ACTION_EDIT_SCALE_EAST,
    ACTION_EDIT_SCALE_NORTH,
    ACTION_EDIT_SCALE_SOUTH,
    ACTION_EDIT_SCALE_DELTA_INCREASE,
    ACTION_EDIT_SCALE_DELTA_DECREASE,

    ACTION_EDIT_MATERIAL_NEXT,
    ACTION_EDIT_MATERIAL_PREVIOUS,
    ACTION_EDIT_MATERIAL_NEXT_PACK,

    ACTION_EDIT_MESH_NEXT,
    ACTION_EDIT_MESH_PREVIOUS,
    ACTION_EDIT_MESH_NEXT_PACK,
    ACTION_EDIT_BBOX_RECALCULATE,

    ACTION_COUNT
};

struct blob_index
{
    enum RICO_hnd_type type;
    u32 name_hash;
    u32 offset;
    u32 min_size;
};

// Memory layout
// -----------------------
// struct pack;
// struct pack_entry index[index_count];
// struct pack_entry fast_index[index_count];
// u8 data[data_size];
struct pack
{
    char magic[4];
    u32 version;
    u32 id;
    buf32 name;

    u32 blob_current_id;
    u32 blob_count;
    u32 blobs_used;

    u32 buffer_size;
    u32 buffer_used;

    u32 lookup_offset;
    u32 index_offset;
    u32 data_offset;

    u32 *lookup;
    struct blob_index *index;
    u8 *buffer;
};

enum PACK_IDS
{
    PACK_DEFAULT,
    PACK_TRANSIENT,
    PACK_FRAME,
    PACK_COUNT
};

enum DEFAULT_IDS
{
    FONT_DEFAULT = 1,
    FONT_DEFAULT_TEXTURE,
    TEXTURE_DEFAULT_DIFF,
    TEXTURE_DEFAULT_SPEC,
    TEXTURE_DEFAULT_EMIS,
    MATERIAL_DEFAULT,
    MESH_DEFAULT_CUBE,
    MESH_DEFAULT_SPHERE
};

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

#define RICO_ERRORS(f)             \
    f(SUCCESS)                     \
    f(ERR_BAD_ALLOC)               \
    f(ERR_POOL_OUT_OF_MEMORY)      \
    f(ERR_POOL_INVALID_HANDLE)     \
    f(ERR_POOL_BAD_FREE)           \
    f(ERR_FILE_WRITE)              \
    f(ERR_FILE_READ)               \
    f(ERR_FILE_SIGNATURE)          \
    f(ERR_FILE_VERSION)            \
    f(ERR_SERIALIZE_DISABLED)      \
    f(ERR_SERIALIZER_NULL)         \
    f(ERR_DESERIALIZER_NULL)       \
    f(ERR_TEXTURE_LOAD)            \
    f(ERR_TEXTURE_UNSUPPORTED_BPP) \
    f(ERR_SHADER_COMPILE)          \
    f(ERR_SHADER_LINK)             \
    f(ERR_SDL_INIT)                \
    f(ERR_GL3W_INIT)               \
    f(ERR_OPENAL_INIT)             \
    f(ERR_FREETYPE_INIT)           \
    f(ERR_PRIM_UNSUPPORTED)        \
    f(ERR_OBJ_TOO_MANY_VERTS)      \
    f(ERR_CHUNK_NULL)              \
    f(ERR_MESH_INVALID_NAME)       \
    f(ERR_TEXTURE_INVALID_NAME)    \
    f(ERR_MATERIAL_INVALID_NAME)   \
    f(ERR_OBJECT_INVALID_NAME)     \
    f(ERR_STRING_INVALID_NAME)     \
    f(ERR_FONT_INVALID_NAME)       \
    f(ERR_HASH_TABLE_FULL)         \
    f(ERR_HASH_INVALID_KEY)        \
    f(ERR_HASH_OVERWRITE)          \
    f(ERR_INVALID_PARAMS)          \
    f(ERR_CHUNK_FREE_FAILED)       \
    f(ERR_OBJ_PARSE_FAILED)        \
    f(ERR_FREETYPE_FACE)           \
    f(ERR_FREETYPE_SIZE)           \
    f(ERR_FREETYPE_CHAR)           \
    f(ERR_COUNT)

enum RICO_error { RICO_ERRORS(GEN_LIST) };
extern const char *RICO_error_string[];

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
    struct vec4 color_bg;
    float percent;
    struct RICO_heiro_string *heiro;
    struct ui_tooltip *tooltip;
};

struct RICO_transform
{
    struct vec3 position;
    struct quat orientation;
    struct vec3 scale;
    struct mat4 matrix;
    struct mat4 matrix_inverse;
};

struct RICO_object
{
    struct uid uid;
    u32 type;
    struct RICO_transform xform;

    // TODO: Replace bbox with aabb
    //struct RICO_aabb aabb;
    //struct RICO_aabb aabb_world;
    struct RICO_bbox bbox;
    struct RICO_bbox bbox_world;
    struct RICO_obb obb;
    struct sphere sphere;

    // TODO: Refactor into physics?
    struct vec3 acc;
    struct vec3 vel;
    bool resting;
    bool collide_sphere;
    bool collide_aabb;
    bool collide_obb;

    bool selected;
    bool select_ignore;

    pkid mesh_id;
    pkid material_id;
};

#define WIDGET_ACTIONS(f) \
    f(WIDGET_NONE)        \
    f(WIDGET_TRANSLATE_X) \
    f(WIDGET_TRANSLATE_Y) \
    f(WIDGET_TRANSLATE_Z)

enum widget_action
{
    WIDGET_ACTIONS(GEN_LIST)
};
extern const char *widget_action_string[];

#define RICO_FILE_VERSION_CURRENT 1
#define RICO_FILE_VERSION_MINIMUM_SUPPORTED 1
#define RICO_FILE_VERSION_MAXIMUM_SUPPORTED RICO_FILE_VERSION_CURRENT
#define RICO_FILE_VERSION_COUNT 1 + (RICO_FILE_VERSION_MAXIMUM_SUPPORTED -\
                                     RICO_FILE_VERSION_MINIMUM_SUPPORTED)

struct rico_file
{
    FILE *fs;
    u32 version;
    u32 next_uid;
    const char *filename;
};

#define WIDTH_DATA_OFFSET  20  // Offset to width data with BFF file
#define MAP_DATA_OFFSET   276  // Offset to texture image data with BFF file

#define BFG_RS_NONE  0x0  // Blend flags
#define BFG_RS_ALPHA 0x1
#define BFG_RS_RGB   0x2
#define BFG_RS_RGBA  0x4

// HACK: Hard-coded font width/height
// TODO: Figure out how to determine font w/h in screen space
#define FONT_WIDTH 8.0f
#define FONT_HEIGHT 16.0f

#define BFG_MAXSTRING 511  // Maximum string length

struct RICO_font
{
    struct uid uid;
    u32 cell_x;
    u32 cell_y;
    u8 base_char;
    u32 row_pitch;
    float col_factor;
    float row_factor;
    s32 y_offset;
    bool y_invert;  // Cleanup: This shit don't do shit.
    u8 render_style;
    u8 char_widths[256];
    pkid tex_id;
};

#define HEIRO_GLYPH_HEIGHT 14

struct RICO_heiro_glyph
{
    u32 width;
    u32 height;
    s32 bearing_left;
    s32 bearing_top;
    s32 advance_x;
    s32 advance_y;

    GLuint gl_id;
    struct vec2 uvs[2];
};

// TODO: Material describes texture(s), device state settings, and which v/f
//       shader to use when rendering.
struct RICO_material
{
    struct uid uid;

    //TODO: vert_shader, frag_shader

    pkid tex_albedo;
    pkid tex_mrao;
    pkid tex_emission;
    // TODO: pkid tex_normal;
};

enum rico_vbo
{
    VBO_VERTEX,
    VBO_ELEMENT,
    VBO_COUNT
};

struct rgl_mesh
{
    GLuint vao;
    GLuint vbos[2];
    u32 vertices;
    u32 elements;
    struct rgl_mesh *next;
};

struct rico_physics
{
    struct vec3 min_size;
    struct vec3 pos;
    struct vec3 vel;
    struct vec3 acc;
};

struct RICO_string
{
    struct uid uid;
    u32 slot;
    u32 lifespan;

    pkid mesh_id;
    pkid texture_id;
};

struct rgl_texture
{
    GLuint gl_id;
    GLenum gl_target;
    GLenum format_internal;
    GLenum format;
    struct rgl_texture *next;
};

struct RICO_texture
{
    struct uid uid;
    u32 width;
    u32 height;
    u8 bpp;
    GLenum gl_target;

    // TODO: Remove this field. Load data directly into VRAM then free buffer.
    u32 pixels_offset;
};

#endif