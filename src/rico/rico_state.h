#ifndef RICO_STATE_H
#define RICO_STATE_H

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

#define CHORD_UP_REPEAT3(k0, k1, k2) (struct RICO_keychord) {{ k0, k1, k2 }, 1, 1 }
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

#define RICO_STATES(f)       \
    f(STATE_PLAY_EXPLORE)    \
    f(STATE_EDIT_TRANSLATE)  \
    f(STATE_EDIT_ROTATE)     \
    f(STATE_EDIT_SCALE)      \
    f(STATE_EDIT_MATERIAL)   \
    f(STATE_EDIT_MESH)       \
    f(STATE_MENU_QUIT)       \
    f(STATE_TEXT_INPUT)      \
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
    ACTION_EDIT_BBOX_RECALCULATE,
    ACTION_EDIT_CREATE_OBJECT,
    ACTION_EDIT_SELECTED_DUPLICATE,
    ACTION_EDIT_SELECTED_DELETE,
    ACTION_EDIT_MODE_NEXT,
    ACTION_EDIT_MODE_PREVIOUS,

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
    ACTION_EDIT_ROTATE_UP,
    ACTION_EDIT_ROTATE_DOWN,
    ACTION_EDIT_ROTATE_WEST,
    ACTION_EDIT_ROTATE_EAST,
    ACTION_EDIT_ROTATE_NORTH,
    ACTION_EDIT_ROTATE_SOUTH,
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
    ACTION_EDIT_MESH_BBOX_RECALCULATE,

    ACTION_COUNT
};

extern enum rico_state RICO_state_get();
extern bool RICO_state_is_paused();
extern bool RICO_state_is_edit();
extern int RICO_update();
extern void RICO_render_objects();
extern void RICO_render_editor();
extern void RICO_render_ui();
extern void RICO_render();
extern void RICO_frame_swap();
extern bool RICO_quit();
extern u32 RICO_key_event(u32 *action);
extern void RICO_bind_action(u32 action, struct RICO_keychord chord);
extern bool RICO_mouse_raycast(pkid *_obj_id, float *_dist);

#endif
