#ifndef RICO_TYPES_H
#define RICO_TYPES_H

////////////////////////////////////////////////////////////////////////////////

#define RIC_ASSET_TYPES(f) \
    f(RIC_ASSET_NULL,      0) \
    f(RIC_ASSET_OBJECT,    sizeof(struct RICO_object))     \
    f(RIC_ASSET_TEXTURE,   sizeof(struct RICO_texture))    \
    f(RIC_ASSET_MESH,      sizeof(struct RICO_mesh))		\
    f(RIC_ASSET_FONT,      sizeof(struct RICO_font))		\
    f(RIC_ASSET_STRING,    sizeof(struct RICO_string))	    \
    f(RIC_ASSET_MATERIAL,  sizeof(struct RICO_material))	\
    f(RIC_ASSET_BBOX,      sizeof(struct RICO_bbox))

enum ric_asset_type
{
    RIC_ASSET_TYPES(ENUM)
    RIC_ASSET_TYPE_COUNT
};
extern const char *ric_asset_type_string[];
extern const u32 ric_asset_type_size[];

#define RIC_AUDIO_STATES(f) \
    f(RICO_AUDIO_UNKNOWN)    \
    f(RICO_AUDIO_STOPPED)    \
    f(RICO_AUDIO_PLAYING)    \
    f(RICO_AUDIO_PAUSED)

enum ric_audio_state
{
    RIC_AUDIO_STATES(ENUM)
    RICO_AUDIO_STATE_COUNT
};
extern const char *ric_audio_state_string[];

#define RIC_LIGHT_TYPES(f) \
    f(RIC_LIGHT_AMBIENT)        \
    f(RIC_LIGHT_DIRECTIONAL)    \
    f(RIC_LIGHT_POINT)          \
    f(RIC_LIGHT_SPOT)

enum ric_light_type
{
    RIC_LIGHT_TYPES(ENUM)
    RIC_LIGHT_TYPE_COUNT
};
extern const char *ric_light_type_string[];

#define RIC_ENGINE_STATES(f) \
    f(RIC_ENGINE_PLAY_EXPLORE)   \
    f(RIC_ENGINE_EDIT_TRANSLATE) \
    f(RIC_ENGINE_EDIT_ROTATE)    \
    f(RIC_ENGINE_EDIT_SCALE)     \
    f(RIC_ENGINE_EDIT_MATERIAL)  \
    f(RIC_ENGINE_EDIT_MESH)      \
    f(RIC_ENGINE_MENU_QUIT)      \
    f(RIC_ENGINE_TEXT_INPUT)     \
    f(RIC_ENGINE_SHUTDOWN)

enum ric_state
{
    RIC_ENGINE_STATES(ENUM)
    RIC_ENGINE_STATE_COUNT
};
extern const char *ric_engine_state_string[RIC_ENGINE_STATE_COUNT];

enum ric_action
{
    RIC_ACTION_NULL,

    RIC_ACTION_ENGINE_DEBUG_LIGHTING_TOGGLE,
    RIC_ACTION_ENGINE_DEBUG_TRIGGER_BREAKPOINT,
    RIC_ACTION_ENGINE_DEBUG_FOV_INCREASE,
    RIC_ACTION_ENGINE_DEBUG_FOV_DECREASE,
    RIC_ACTION_ENGINE_FPS_TOGGLE,
    RIC_ACTION_ENGINE_MOUSE_LOCK_TOGGLE,
    RIC_ACTION_ENGINE_VSYNC_TOGGLE,
    RIC_ACTION_ENGINE_MUTE_TOGGLE,
    RIC_ACTION_ENGINE_SIM_PAUSE,
    RIC_ACTION_ENGINE_QUIT,

    RIC_ACTION_CAMERA_SLOW_TOGGLE,
    RIC_ACTION_CAMERA_RESET,
    RIC_ACTION_CAMERA_LOCK_TOGGLE,
    RIC_ACTION_CAMERA_WIREFRAME_TOGGLE,

    RIC_ACTION_MOVE_RIGHT,
    RIC_ACTION_MOVE_LEFT,
    RIC_ACTION_MOVE_UP,
    RIC_ACTION_MOVE_DOWN,
    RIC_ACTION_MOVE_FORWARD,
    RIC_ACTION_MOVE_BACKWARD,
    RIC_ACTION_MOVE_SPRINT,

    RIC_ACTION_PLAY_EDITOR,
    RIC_ACTION_PLAY_INTERACT,

    RIC_ACTION_EDIT_QUIT,
    RIC_ACTION_EDIT_SAVE,
    RIC_ACTION_EDIT_CYCLE_PACK,
    RIC_ACTION_EDIT_CYCLE_BLOB_REVERSE,
    RIC_ACTION_EDIT_CYCLE_BLOB,
    RIC_ACTION_EDIT_MOUSE_PICK_START,
    RIC_ACTION_EDIT_MOUSE_PICK_MOVE,
    RIC_ACTION_EDIT_MOUSE_PICK_END,
    RIC_ACTION_EDIT_BBOX_RECALCULATE_ALL,
    RIC_ACTION_EDIT_CREATE_OBJECT,
    RIC_ACTION_EDIT_SELECTED_DUPLICATE,
    RIC_ACTION_EDIT_SELECTED_DELETE,
    RIC_ACTION_EDIT_MODE_PREVIOUS,
    RIC_ACTION_EDIT_MODE_NEXT,
    RIC_ACTION_EDIT_CAMERA_TOGGLE_PROJECTION,
    RIC_ACTION_EDIT_DEBUG_TEXT_INPUT,

    RIC_ACTION_EDIT_TRANSLATE_RESET,
    RIC_ACTION_EDIT_TRANSLATE_UP,
    RIC_ACTION_EDIT_TRANSLATE_DOWN,
    RIC_ACTION_EDIT_TRANSLATE_WEST,
    RIC_ACTION_EDIT_TRANSLATE_EAST,
    RIC_ACTION_EDIT_TRANSLATE_NORTH,
    RIC_ACTION_EDIT_TRANSLATE_SOUTH,
    RIC_ACTION_EDIT_TRANSLATE_DELTA_INCREASE,
    RIC_ACTION_EDIT_TRANSLATE_DELTA_DECREASE,

    RIC_ACTION_EDIT_ROTATE_RESET,
    RIC_ACTION_EDIT_ROTATE_X_POS,
    RIC_ACTION_EDIT_ROTATE_X_NEG,
    RIC_ACTION_EDIT_ROTATE_Y_POS,
    RIC_ACTION_EDIT_ROTATE_Y_NEG,
    RIC_ACTION_EDIT_ROTATE_Z_POS,
    RIC_ACTION_EDIT_ROTATE_Z_NEG,
    RIC_ACTION_EDIT_ROTATE_DELTA_INCREASE,
    RIC_ACTION_EDIT_ROTATE_DELTA_DECREASE,
    RIC_ACTION_EDIT_ROTATE_HEPTAMODE_TOGGLE,

    RIC_ACTION_EDIT_SCALE_RESET,
    RIC_ACTION_EDIT_SCALE_UP,
    RIC_ACTION_EDIT_SCALE_DOWN,
    RIC_ACTION_EDIT_SCALE_WEST,
    RIC_ACTION_EDIT_SCALE_EAST,
    RIC_ACTION_EDIT_SCALE_NORTH,
    RIC_ACTION_EDIT_SCALE_SOUTH,
    RIC_ACTION_EDIT_SCALE_DELTA_INCREASE,
    RIC_ACTION_EDIT_SCALE_DELTA_DECREASE,

    RIC_ACTION_EDIT_MATERIAL_NEXT,
    RIC_ACTION_EDIT_MATERIAL_PREVIOUS,
    RIC_ACTION_EDIT_MATERIAL_NEXT_PACK,

    RIC_ACTION_EDIT_MESH_NEXT,
    RIC_ACTION_EDIT_MESH_PREVIOUS,
    RIC_ACTION_EDIT_MESH_NEXT_PACK,
    RIC_ACTION_EDIT_BBOX_RECALCULATE,

    RIC_ACTION_COUNT
};

// TODO: Replace with proper temp / frame arenas.
enum ric_pack_ids
{
    RIC_PACK_ID_DEFAULT,
    RIC_PACK_ID_TRANSIENT,
    RIC_PACK_ID_FRAME,
    RIC_PACK_ID_COUNT
};

#define RIC_STRING_SLOTS(f) \
    f(RIC_STRING_SLOT_SELECTED_OBJ) \
    f(RIC_STRING_SLOT_STATE)        \
    f(RIC_STRING_SLOT_FPS)          \
    f(RIC_STRING_SLOT_MENU_QUIT)    \
    f(RIC_STRING_SLOT_DELTA)        \
    f(RIC_STRING_SLOT_WIDGET)       \
    f(RIC_STRING_SLOT_DEBUG_CAMERA)

enum ric_string_slot
{
    RIC_STRING_SLOTS(ENUM)
    RIC_STRING_SLOT_COUNT
};
extern const char *ric_string_slot_string[];

#define RIC_ERRORS(f) \
    f(RIC_SUCCESS)                     \
    f(RIC_ERR_BAD_ALLOC)               \
    f(RIC_ERR_POOL_OUT_OF_MEMORY)      \
    f(RIC_ERR_POOL_INVALID_HANDLE)     \
    f(RIC_ERR_POOL_BAD_FREE)           \
    f(RIC_ERR_FILE_WRITE)              \
    f(RIC_ERR_FILE_READ)               \
    f(RIC_ERR_FILE_SIGNATURE)          \
    f(RIC_ERR_FILE_VERSION)            \
    f(RIC_ERR_SERIALIZE_DISABLED)      \
    f(RIC_ERR_SERIALIZER_NULL)         \
    f(RIC_ERR_DESERIALIZER_NULL)       \
    f(RIC_ERR_TEXTURE_LOAD)            \
    f(RIC_ERR_TEXTURE_UNSUPPORTED_BPP) \
    f(RIC_ERR_SHADER_COMPILE)          \
    f(RIC_ERR_SHADER_LINK)             \
    f(RIC_ERR_SDL_INIT)                \
    f(RIC_ERR_GL3W_INIT)               \
    f(RIC_ERR_OPENAL_INIT)             \
    f(RIC_ERR_FREETYPE_INIT)           \
    f(RIC_ERR_PRIM_UNSUPPORTED)        \
    f(RIC_ERR_OBJ_TOO_MANY_VERTS)      \
    f(RIC_ERR_CHUNK_NULL)              \
    f(RIC_ERR_MESH_INVALID_NAME)       \
    f(RIC_ERR_TEXTURE_INVALID_NAME)    \
    f(RIC_ERR_MATERIAL_INVALID_NAME)   \
    f(RIC_ERR_OBJECT_INVALID_NAME)     \
    f(RIC_ERR_STRING_INVALID_NAME)     \
    f(RIC_ERR_FONT_INVALID_NAME)       \
    f(RIC_ERR_HASH_TABLE_FULL)         \
    f(RIC_ERR_HASH_INVALID_KEY)        \
    f(RIC_ERR_HASH_OVERWRITE)          \
    f(RIC_ERR_INVALID_PARAMS)          \
    f(RIC_ERR_CHUNK_FREE_FAILED)       \
    f(RIC_ERR_OBJ_PARSE_FAILED)        \
    f(RIC_ERR_FREETYPE_FACE)           \
    f(RIC_ERR_FREETYPE_SIZE)           \
    f(RIC_ERR_FREETYPE_CHAR)

enum ric_error
{
    RIC_ERRORS(ENUM)
    RIC_ERR_COUNT
};
extern const char *ric_error_string[];

enum ric_ui_type
{
    RIC_UI_HUD,
    RIC_UI_BREAK,
    RIC_UI_STRING,
    RIC_UI_BUTTON,
    RIC_UI_LABEL,
    RIC_UI_PROGRESS,
    RIC_UI_COUNT
};

enum ric_ui_event_type
{
    RIC_UI_EVENT_HOVER,
    RIC_UI_EVENT_LMB_CLICK,
    RIC_UI_EVENT_LMB_DOWN,
    RIC_UI_EVENT_LMB_UP,
    RIC_UI_EVENT_MMB_CLICK,
    RIC_UI_EVENT_MMB_DOWN,
    RIC_UI_EVENT_MMB_UP,
    RIC_UI_EVENT_RMB_CLICK,
    RIC_UI_EVENT_RMB_DOWN,
    RIC_UI_EVENT_RMB_UP
};

enum ric_ui_state
{
    RIC_UI_STATE_DEFAULT,
    RIC_UI_STATE_HOVERED,
    RIC_UI_STATE_PRESSED,
    RIC_UI_STATE_COUNT
};

#define RIC_WIDGETS(f) \
    f(RIC_WIDGET_TRANSLATE_X) \
    f(RIC_WIDGET_TRANSLATE_Y) \
    f(RIC_WIDGET_TRANSLATE_Z)

enum ric_widget
{
    RIC_WIDGETS(ENUM)
    RIC_WIDGET_COUNT
};
extern const char *ric_widget_string[];

enum ric_vbo_type
{
    RIC_VBO_VERTEX,
    RIC_VBO_ELEMENT,
    RIC_VBO_COUNT
};

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

typedef u8 buf32[32];

struct uid
{
    pkid pkid;
    enum ric_asset_type type;
    buf32 name;
};

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

struct RICO_light
{
    enum ric_light_type type;
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

struct blob_index
{
    enum ric_asset_type type;
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

struct RICO_ui_event
{
    struct RICO_ui_element *element;
    enum ric_ui_event_type event_type;
};

typedef void (*RICO_ui_event_handler)(const struct RICO_ui_event *e);

struct RICO_ui_head
{
    enum ric_ui_type type;
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

struct RICO_ui_button
{
    struct RICO_ui_element element;
    enum ric_ui_state state;
    struct vec4 color[RIC_UI_STATE_COUNT];
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
    enum ric_ui_state state;
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