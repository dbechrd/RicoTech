#define LOAD_SAVE_FILE false

///|////////////////////////////////////////////////////////////////////////////
const char *rico_state_string[] = {
    RICO_STATES(GEN_STRING)
};

enum rico_action {
    ACTION_ENGINE_DEBUG_LIGHTING_TOGGLE,
    ACTION_ENGINE_DEBUG_TRIGGER_BREAKPOINT,
    ACTION_ENGINE_EDITOR_TOGGLE,
    ACTION_ENGINE_FPS_TOGGLE,
    ACTION_ENGINE_MOUSE_LOCK_TOGGLE,
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

    ACTION_EDIT_SAVE,
    ACTION_EDIT_CYCLE,
    ACTION_EDIT_CYCLE_REVERSE,
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

    ACTION_EDIT_MESH_NEXT,
    ACTION_EDIT_MESH_PREVIOUS,
    ACTION_EDIT_MESH_BBOX_RECALCULATE,

    ACTION_COUNT
};

typedef u8 rico_key;

struct rico_keychord {
    rico_key keys[3];
};

#define CHORD3(k0, k1, k2) (struct rico_keychord) {{ k0, k1, k2 }};
#define CHORD2(k0, k1) CHORD3(k0, k1, 0)
#define CHORD1(k0)     CHORD3(k0, 0, 0)

#if 0
struct rico_keysequence {
    union {
        struct rico_keychord chords[3];
        struct {
            struct rico_keychord c0;
            struct rico_keychord c1;
            struct rico_keychord c2;
        };
    };
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
    union {
        rico_key key;
        struct rico_keychord chord;
        struct rico_keysequence sequence;
    };
};

#define EVENT (struct rico_keyevent)
#define EVENT_KEY(key) EVENT{ RICO_KEYEVENT_KEY, key }
#define EVENT_CHORD(k0, k1) EVENT{ RICO_KEYEVENT_CHORD, k0, k1 }
#endif

static struct rico_keychord action_chords[ACTION_COUNT] = { 0 };

///|////////////////////////////////////////////////////////////////////////////
//Human walk speed empirically found to be 33 steps in 20 seconds. That is
// approximately 1.65 steps per second. At 60 fps, that is 0.0275 steps per
// frame. Typical walking stride is ~0.762 meters (30 inches). Distance traveled
// per frame (60hz) is 0.762 * 0.0275 = 0.020955 ~= 0.021

struct { int x, y; } mouse_delta = { 0 };
global float look_sensitivity_x = 0.1f;
global float look_sensitivity_y = 0.1f;

global struct vec3 view_vel;
global struct vec3 view_acc;
global float player_acceleration = 20.0f;
global float sprint_factor = 2.0f;
global bool sprint = false;
global float friction_factor = 0.1f;

global float camera_slow_factor = 0.1f;
global bool camera_slow = false;

///|////////////////////////////////////////////////////////////////////////////
#define TRANS_DELTA_MIN 0.01f
#define TRANS_DELTA_MAX 10.0f
global float trans_delta = 1.0f;

#define ROT_DELTA_MIN 1.0f
#define ROT_DELTA_MAX 90.0f
#define ROT_DELTA_DEFAULT 5.0f
global float rot_delta = ROT_DELTA_DEFAULT;

#define SCALE_DELTA_MIN 0.1f
#define SCALE_DELTA_MAX 5.0f
global float scale_delta = 1.0f;

global bool mouse_lock = true;
global bool enable_lighting = true;

///|////////////////////////////////////////////////////////////////////////////
// Performance timing
global u64 perfs_frequency;
global u64 last_perfs;
global u64 last_cycles;

// FPS UI
global u64 fps_last_render;
global u64 fps_render_delta = 1; //200000;  // 200 ms
global bool fps_render = false;

///|////////////////////////////////////////////////////////////////////////////
// Mouse and keyboard state
global u32 mouse_buttons = 0;
global u32 mouse_buttons_prev = 0;

global u8 keystate_buffers[2][SDL_NUM_SCANCODES] = { 0 };
global u8 *keys      = keystate_buffers[0];
global u8 *keys_prev = keystate_buffers[1];

const u8 RICO_SCANCODE_ALT   = 1;
const u8 RICO_SCANCODE_CTRL  = 2;
const u8 RICO_SCANCODE_SHIFT = 3;

#define KEY_IS_DOWN(key) (key == 0 || keys[key] || \
(key == RICO_SCANCODE_ALT   && (keys[SDL_SCANCODE_LALT]   || keys[SDL_SCANCODE_RALT]))  || \
(key == RICO_SCANCODE_CTRL  && (keys[SDL_SCANCODE_LCTRL]  || keys[SDL_SCANCODE_RCTRL])) || \
(key == RICO_SCANCODE_SHIFT && (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])))

#define KEY_WAS_DOWN(key) (key == 0 || keys_prev[key] || \
(key == RICO_SCANCODE_ALT   && (keys_prev[SDL_SCANCODE_LALT]   || keys_prev[SDL_SCANCODE_RALT]))  || \
(key == RICO_SCANCODE_CTRL  && (keys_prev[SDL_SCANCODE_LCTRL]  || keys_prev[SDL_SCANCODE_RCTRL])) || \
(key == RICO_SCANCODE_SHIFT && (keys_prev[SDL_SCANCODE_LSHIFT] || keys_prev[SDL_SCANCODE_RSHIFT])))

#define KEY_PRESSED(key)  ( KEY_IS_DOWN(key) && !KEY_WAS_DOWN(key))
#define KEY_RELEASED(key) (!KEY_IS_DOWN(key) &&  KEY_WAS_DOWN(key))

static inline bool chord_is_down(enum rico_action action)
{
    RICO_ASSERT(action_chords[action].keys[0] ||
                action_chords[action].keys[1] ||
                action_chords[action].keys[2]);

    // All keys are down
    return (KEY_IS_DOWN(action_chords[action].keys[0]) &&
            KEY_IS_DOWN(action_chords[action].keys[1]) &&
            KEY_IS_DOWN(action_chords[action].keys[2]));
}
static inline bool chord_was_down(enum rico_action action)
{
    RICO_ASSERT(action_chords[action].keys[0] ||
                action_chords[action].keys[1] ||
                action_chords[action].keys[2]);

    // All keys were down last frame
    return (KEY_WAS_DOWN(action_chords[action].keys[0]) &&
            KEY_WAS_DOWN(action_chords[action].keys[1]) &&
            KEY_WAS_DOWN(action_chords[action].keys[2]));
}
static inline bool chord_pressed(enum rico_action action)
{
    // All keys are down, and at least one was up last frame
    return chord_is_down(action) && !chord_was_down(action);
}
static inline bool chord_released(enum rico_action action)
{
    // At least one key is up, and all were down last frame
    return !chord_is_down(action) && chord_was_down(action);
}

// TODO: Where should these functions go?
internal int save_file()
{
    RICO_ASSERT(chunk_active);
    enum rico_error err;

    if (chunk_active->uid == UID_NULL)
        return RICO_ERROR(ERR_CHUNK_NULL, "Failed to save NULL chunk");

    struct rico_file file;
    err = rico_file_open_write(&file, "chunks/cereal.bin",
                               RICO_FILE_VERSION_CURRENT);
    if (err) return err;

    err = chunk_serialize(chunk_active, &file);
    rico_file_close(&file);

#if RICO_SAVE_BACKUP
    // Save backup copy
    time_t rawtime = time(NULL);
    struct tm *tm = localtime(&rawtime);

    char backupFilename[128] = { 0 };
    int len = snprintf(backupFilename, sizeof(backupFilename),
                       "chunks/cereal_%04d%02d%02dT%02d%02d%02d.bin",
                       1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday,
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
    string_truncate(backupFilename, sizeof(backupFilename), len);

    struct rico_file backupFile;
    err = rico_file_open_write(&backupFile, backupFilename,
                               RICO_FILE_VERSION_CURRENT);
    if (err) return err;

    err = rico_serialize(chunk, &backupFile);
    rico_file_close(&backupFile);
#endif

    return err;
}
internal void select_first_obj()
{
    struct rico_object *obj_collided = { 0 };
    float dist;

    // Camera forward ray v. scene
    struct ray cam_fwd;
    camera_fwd(&cam_fwd, &cam_player);
    object_collide_ray_type(pack_active, &obj_collided, &dist, OBJ_STATIC,
                            &cam_fwd);
    select_obj(obj_collided);
}

// Current state
global enum rico_state state_prev;
global enum rico_state state;

// State change handlers
typedef int (*state_handler)();
struct state_handlers {
    state_handler init;
    state_handler run;
    state_handler cleanup;
};
struct state_handlers state_handlers[STATE_COUNT] = { 0 };

internal inline enum rico_state state_get()
{
    return state;
}
internal inline bool is_edit_state(enum rico_state state)
{
    return (state == STATE_EDIT_TRANSLATE ||
            state == STATE_EDIT_ROTATE ||
            state == STATE_EDIT_SCALE ||
            state == STATE_EDIT_TEXTURE ||
            state == STATE_EDIT_MESH);
}
internal inline bool is_paused_state(enum rico_state state)
{
    return state == STATE_MENU_QUIT;
}

// TODO: Move this to camera file
void temp_camera_update(r64 dt)
{
    // Calculate delta velocity
    // dv' = at;
    struct vec3 delta_vel = view_acc;
    v3_scalef(&delta_vel, player_acceleration);
    if (sprint)      v3_scalef(&delta_vel, sprint_factor);
    if (camera_slow) v3_scalef(&delta_vel, camera_slow_factor);
    v3_scalef(&delta_vel, (r32)dt);

    // Update velocity
    v3_add(&view_vel, &delta_vel);

    // Apply friction (double when slowing down for a more realistic stop)
    float mag_acc = v3_length(&view_acc);
    if (mag_acc != 0.0f)
        v3_scalef(&view_vel, 1.0f - friction_factor);
    else
        v3_scalef(&view_vel, 1.0f - friction_factor * 2.0f);

    // Resting check
    float mag_vel = v3_length(&view_vel);
    if (mag_vel < VEC3_EPSILON)
    {
        view_vel = VEC3_ZERO;
    }

#if 0
    // DEBUG: Print velocity
    char buf[64] = { 0 };
    int len = snprintf(buf, sizeof(buf), "Vel: %.1f %.1f %.1f", view_vel.x,
                       view_vel.y, view_vel.z);
    string_truncate(buf, sizeof(buf), len);

    struct rico_string *str;
    err = chunk_alloc(chunk_transient, RICO_HND_STRING, (struct hnd **)&str);
    if (err) return err;
    err = string_init(str, RICO_TRANSIENT, "STR_VEL", STR_SLOT_DEBUG, 0,
                      -FONT_HEIGHT, COLOR_DARK_RED_HIGHLIGHT, 0, NULL,
                      buf);
    if (err) return err;
#endif

    // Calculate delta position
    // dp' = 1/2at^2 + vt
    struct vec3 half_at_squared = view_acc;
    v3_scalef(&half_at_squared, 0.5f * (r32)dt * (r32)dt);

    struct vec3 vt = view_vel;
    v3_scalef(&vt, (r32)dt);

    struct vec3 delta_pos = half_at_squared;
    v3_add(&delta_pos, &vt);

    if (mouse_lock)
    {
        // Update position
        camera_translate_local(&cam_player, &delta_pos);

        // TODO: Smooth mouse look somehow
        if (mouse_delta.x != 0 || mouse_delta.y != 0)
        {
            struct vec3 delta;
            delta.x = mouse_delta.x * look_sensitivity_x;
            delta.y = mouse_delta.y * look_sensitivity_y;
            camera_rotate(&cam_player, delta.x, delta.y);
        }
    }

    camera_update(&cam_player);
}

/*
internal void clear_slot_string(enum rico_string_slot slot)
{
    // TODO: How to make this more logical? Maybe STR_SLOT_* should be handles?
    const char *slot_name = rico_string_slot_string[slot];
    struct rico_string *str;
    str = hashtable_search_str(&global_strings, slot_name);
    if (str)
    {
        string_free(str);
        hashtable_delete_str(&global_strings, slot_name);
    }
}
*/

int state_update()
{
    enum rico_error err;

    ///-------------------------------------------------------------------------
    //| Query input state
    ///-------------------------------------------------------------------------
    // Get mouse state
    mouse_buttons_prev = mouse_buttons;
    mouse_buttons = SDL_GetRelativeMouseState(&mouse_delta.x, &mouse_delta.y);

    // Get keyboard state
    u8 *keys_tmp = keys_prev;
    keys_prev = keys;
    keys = keys_tmp;
    memcpy(keys, SDL_GetKeyboardState(0), SDL_NUM_SCANCODES);

    ///-------------------------------------------------------------------------
    //| State actions
    ///-------------------------------------------------------------------------
    state_prev = state;

    err = state_handlers[state].run();
    if (err) return err;

    if (state == STATE_ENGINE_SHUTDOWN)
        return SUCCESS;

    if (state != state_prev)
    {
        // Clean up old state
        if (state_handlers[state_prev].cleanup)
        {
            err = state_handlers[state_prev].cleanup();
            if (err) return err;
        }
        // Initialize new state
        if (state_handlers[state].init)
        {
            err = state_handlers[state].init();
            if (err) return err;
        }

        char buf[32] = { 0 };
        int len = snprintf(buf, sizeof(buf), "State: %d %s", state,
                           rico_state_string[state]);
        string_truncate(buf, sizeof(buf), len);

        string_free_slot(STR_SLOT_STATE);
        struct rico_string *str;
        err = chunk_alloc((void **)&str, chunk_transient, RICO_HND_STRING);
        if (err) return err;
        err = string_init(str, rico_string_slot_string[STR_SLOT_STATE],
                          STR_SLOT_STATE, 0, 0, COLOR_DARK_RED_HIGHLIGHT, 0,
                          NULL, buf);
        if (err) return err;
    }

    ///-------------------------------------------------------------------------
    //| Update time
    ///-------------------------------------------------------------------------
    u64 perfs = SDL_GetPerformanceCounter();
    u64 delta_perfs = (perfs - last_perfs);
    // Smoothing function (weighted running average)
    //frame_ticks = LERP(frame_ticks, new_time - time, 0.01);
    last_perfs = perfs;

    u64 cycles = __rdtsc();
    u64 delta_cycles = (cycles - last_cycles);
    last_cycles = cycles;

    r64 dt = (r64)delta_perfs / perfs_frequency;

    // Update FPS counter string if enabled and overdue
    if (fps_render && last_perfs - fps_last_render > fps_render_delta)
    {
        r64 fps = (r64)perfs_frequency / delta_perfs;
        r64 ms = dt * 1000.0;
        r64 mcyc = (r64)delta_cycles / (1000.0 * 1000.0); // MegaCycles/Frame

        // TODO: string_init() negative x/y coords = backwards from SCREEN_W.
        //       Allow x/y coords to be given in characters instead of pixels
        //       based on FONT_WIDTH / FONT_HEIGHT.
        char buf[128] = { 0 };
        int len = snprintf(buf, sizeof(buf), "%.f fps %.2f ms %.2f mcyc", fps,
                           ms, mcyc);
        string_truncate(buf, sizeof(buf), len);

        string_free_slot(STR_SLOT_FPS);
        struct rico_string *str;
        err = chunk_alloc((void **)&str, chunk_transient, RICO_HND_STRING);
        if (err) return err;
        err = string_init(str, rico_string_slot_string[STR_SLOT_FPS],
                          STR_SLOT_FPS, -(FONT_WIDTH * len), 0,
                          COLOR_DARK_RED_HIGHLIGHT, 0, NULL, buf);
        if (err) return err;

        fps_last_render = last_perfs;
    }

    ///-------------------------------------------------------------------------
    //| Update
    ///-------------------------------------------------------------------------
    if (!is_paused_state(state))
    {
        temp_camera_update(dt);
        glref_update(dt);
    }

    // Update debug text
    string_update(dt);

    ///-------------------------------------------------------------------------
    //| Render
    ///-------------------------------------------------------------------------
    //glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClearColor(0.46f, 0.70f, 1.0f, 1.0f);
    //glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glPolygonMode(GL_FRONT_AND_BACK, cam_player.fill_mode);
    glPolygonMode(GL_FRONT, cam_player.fill_mode);

    glref_render(&cam_player);
    camera_render(&cam_player);

    return err;
}

internal int shared_engine_events()
{
    enum rico_error err = SUCCESS;

    // Toggle FPS counter
    if (chord_pressed(ACTION_ENGINE_FPS_TOGGLE))
    {
        fps_render = !fps_render;
        if (!fps_render)
        {
            string_free_slot(STR_SLOT_FPS);
        }
    }
    // Save and exit
    else if (chord_pressed(ACTION_ENGINE_QUIT))
    {
        string_free_slot(STR_SLOT_MENU_QUIT);
        struct rico_string *str;
        err = chunk_alloc((void **)&str, chunk_transient, RICO_HND_STRING);
        if (err) return err;
        err = string_init(str, rico_string_slot_string[STR_SLOT_MENU_QUIT],
                          STR_SLOT_MENU_QUIT, 600, 400,
                          COLOR_DARK_GREEN_HIGHLIGHT, 0, NULL,
                          "                       \n" \
                          "  Save and quit?       \n" \
                          "                       \n" \
                          "  [Y] Yes              \n" \
                          "  [N] No               \n" \
                          "  [Q] Quit w/o saving  \n" \
                          "                       ");
        if (err) return err;
        state = STATE_MENU_QUIT;
    }
    // Toggle scene lighting
    else if (chord_pressed(ACTION_ENGINE_DEBUG_LIGHTING_TOGGLE))
    {
        // TODO: Pretty sure this is broken
        // TODO: Use this to change shader program on render
        enable_lighting = !enable_lighting;
    }
    // Toggle mouse lock-to-window
    else if (chord_pressed(ACTION_ENGINE_MOUSE_LOCK_TOGGLE))
    {
        mouse_lock = !mouse_lock;
        SDL_SetRelativeMouseMode(mouse_lock);
    }
#if RICO_DEBUG
    // DEBUG: Trigger breakpoint
    else if (chord_pressed(ACTION_ENGINE_DEBUG_TRIGGER_BREAKPOINT))
    {
        SDL_TriggerBreakpoint();
    }
#endif

    return err;
}
internal int shared_camera_events()
{
    enum rico_error err = SUCCESS;

    view_acc = VEC3_ZERO;
    if (chord_is_down(ACTION_MOVE_UP))       view_acc.y += 1.0f;
    if (chord_is_down(ACTION_MOVE_DOWN))     view_acc.y -= 1.0f;
    if (chord_is_down(ACTION_MOVE_RIGHT))    view_acc.x += 1.0f;
    if (chord_is_down(ACTION_MOVE_LEFT))     view_acc.x -= 1.0f;
    if (chord_is_down(ACTION_MOVE_FORWARD))  view_acc.z += 1.0f;
    if (chord_is_down(ACTION_MOVE_BACKWARD)) view_acc.z -= 1.0f;
    v3_normalize(&view_acc);  // NOTE: Prevent faster movement on diagonal

    sprint = chord_is_down(ACTION_MOVE_SPRINT);

    if (chord_pressed(ACTION_CAMERA_SLOW_TOGGLE))
    {
        camera_slow = !camera_slow;
    }
    else if (chord_pressed(ACTION_CAMERA_RESET))
    {
        camera_reset(&cam_player);
    }
    else if (chord_pressed(ACTION_CAMERA_LOCK_TOGGLE))
    {
        cam_player.locked = !cam_player.locked;
    }
    else if (chord_pressed(ACTION_CAMERA_WIREFRAME_TOGGLE))
    {
        cam_player.fill_mode = (cam_player.fill_mode == GL_FILL)
            ? GL_LINE
            : GL_FILL;
    }

    return err;
}
internal int shared_edit_events()
{
    RICO_ASSERT(is_edit_state(state));

    enum rico_error err = SUCCESS;

    // TODO: Refactor mouse check out into chord_pressed() somehow?
    // Raycast object selection
    if (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        select_first_obj();
    }

    // Recalculate bounding boxes of all objects
    if (chord_pressed(ACTION_EDIT_BBOX_RECALCULATE))
    {
        recalculate_all_bbox();
    }
    // Duplicate selected object
    else if (chord_pressed(ACTION_EDIT_SELECTED_DUPLICATE))
    {
        selected_duplicate();
    }
    // Exit edit mode
    else if (chord_pressed(ACTION_ENGINE_EDITOR_TOGGLE))
    {
        state = STATE_PLAY_EXPLORE;
    }
    // Select previous edit mode
    else if (chord_pressed(ACTION_EDIT_MODE_PREVIOUS))
    {
        switch (state)
        {
        case STATE_EDIT_ROTATE:
        case STATE_EDIT_SCALE:
        case STATE_EDIT_TEXTURE:
        case STATE_EDIT_MESH:
            state--;
            break;
        case STATE_EDIT_TRANSLATE:
            state = STATE_EDIT_MESH;
            break;
        default:
            RICO_ASSERT("WTF");
        }
    }
    // Select next edit mode
    else if (chord_pressed(ACTION_EDIT_MODE_NEXT))
    {
        switch (state)
        {
        case STATE_EDIT_TRANSLATE:
        case STATE_EDIT_ROTATE:
        case STATE_EDIT_SCALE:
        case STATE_EDIT_TEXTURE:
            state++;
            break;
        case STATE_EDIT_MESH:
            state = STATE_EDIT_TRANSLATE;
            break;
        default:
            RICO_ASSERT("WTF");
        }
    }
    // Cycle select through objects (in reverse)
    else if (chord_pressed(ACTION_EDIT_CYCLE_REVERSE))
    {
        select_prev_obj();
    }
    // Cycle select through objects
    else if (chord_pressed(ACTION_EDIT_CYCLE))
    {
        select_next_obj();
    }
    // Create new object
    else if (chord_pressed(ACTION_EDIT_CREATE_OBJECT))
    {
        create_obj();
    }
    // Delete selected object
    else if (chord_pressed(ACTION_EDIT_SELECTED_DELETE))
    {
        selected_delete();
    }
    // Save chunk
    else if (chord_pressed(ACTION_EDIT_SAVE))
    {
        save_file();
    }

    return err;
}

internal int state_play_explore()
{
    enum rico_error err = SUCCESS;

    err = shared_engine_events(); if (err || state != state_prev) return err;
    err = shared_camera_events(); if (err || state != state_prev) return err;

    // Enter edit mode
    if (chord_pressed(ACTION_ENGINE_EDITOR_TOGGLE))
    {
        state = STATE_EDIT_TRANSLATE;
        selected_print();
    }

    return err;
}
internal int state_edit_cleanup()
{
    enum rico_error err = SUCCESS;

    if (!is_edit_state(state))
    {
        string_free_slot(STR_SLOT_SELECTED_OBJ);
    }

    return err;
}
internal int state_edit_translate()
{
    enum rico_error err = SUCCESS;

    err = shared_edit_events();   if (err || state != state_prev) return err;
    err = shared_engine_events(); if (err || state != state_prev) return err;
    err = shared_camera_events(); if (err || state != state_prev) return err;

    struct vec3 translate = VEC3_ZERO;
    bool translate_reset = false;
    bool trans_delta_changed = false;

    // Reset selected object's translation
    if (chord_pressed(ACTION_EDIT_TRANSLATE_RESET))
        translate_reset = true;
    // Translate selected object up
    else if (chord_pressed(ACTION_EDIT_TRANSLATE_UP))
        translate.y += trans_delta;
    // Translate selected object down
    else if (chord_pressed(ACTION_EDIT_TRANSLATE_DOWN))
        translate.y -= trans_delta;
    // Translate selected object west
    else if (chord_pressed(ACTION_EDIT_TRANSLATE_WEST))
        translate.x -= trans_delta;
    // Translate selected object east
    else if (chord_pressed(ACTION_EDIT_TRANSLATE_EAST))
        translate.x += trans_delta;
    // Translate selected object north
    else if (chord_pressed(ACTION_EDIT_TRANSLATE_NORTH))
        translate.z -= trans_delta;
    // Translate selected object south
    else if (chord_pressed(ACTION_EDIT_TRANSLATE_SOUTH))
        translate.z += trans_delta;
    // Increase translation delta
    else if (chord_pressed(ACTION_EDIT_TRANSLATE_DELTA_INCREASE))
    {
        if (trans_delta < TRANS_DELTA_MAX)
        {
            trans_delta *= 10.0f;
            if (trans_delta > TRANS_DELTA_MAX)
                trans_delta = TRANS_DELTA_MAX;

            trans_delta_changed = true;
        }
    }
    // Decrease translation delta
    else if (chord_pressed(ACTION_EDIT_TRANSLATE_DELTA_DECREASE))
    {
        if (trans_delta > TRANS_DELTA_MIN)
        {
            trans_delta /= 10.0f;
            if (trans_delta < TRANS_DELTA_MIN)
                trans_delta = TRANS_DELTA_MIN;

            trans_delta_changed = true;
        }
    }

    // Update selected object
    if (translate_reset || !v3_equals(&translate, &VEC3_ZERO))
    {
        selected_translate(&cam_player, &translate);
    }

    if (trans_delta_changed)
    {
        char buf[32] = { 0 };
        int len = snprintf(buf, sizeof(buf), "Trans Delta: %f", trans_delta);
        string_truncate(buf, sizeof(buf), len);

        string_free_slot(STR_SLOT_DELTA);
        struct rico_string *str;
        err = chunk_alloc((void **)&str, chunk_transient, RICO_HND_STRING);
        if (err) return err;
        err = string_init(str, rico_string_slot_string[STR_SLOT_DELTA],
                          STR_SLOT_DELTA, 0, 0, COLOR_DARK_BLUE_HIGHLIGHT,
                          1000, NULL, buf);
        if (err) return err;
    }

    return err;
}
internal int state_edit_rotate()
{
    enum rico_error err = SUCCESS;

    err = shared_edit_events();   if (err || state != state_prev) return err;
    err = shared_engine_events(); if (err || state != state_prev) return err;
    err = shared_camera_events(); if (err || state != state_prev) return err;

    struct vec3 rotate = VEC3_ZERO;
    bool rotate_reset = false;
    bool rot_delta_changed = false;

    // Toggle heptagonal rotations
    if (chord_pressed(ACTION_EDIT_ROTATE_HEPTAMODE_TOGGLE))
    {
        if (rot_delta == ROT_DELTA_DEFAULT)
            rot_delta = (float)M_SEVENTH_DEG;
        else
            rot_delta = ROT_DELTA_DEFAULT;
    }
    // Reset selected object's rotation
    else if (chord_pressed(ACTION_EDIT_ROTATE_RESET))
        rotate_reset = true;
    // Rotate selected object up
    else if (chord_pressed(ACTION_EDIT_ROTATE_UP))
        rotate.x -= rot_delta;
    // Rotate selected object down
    else if (chord_pressed(ACTION_EDIT_ROTATE_DOWN))
        rotate.x += rot_delta;
    // Rotate selected object west
    else if (chord_pressed(ACTION_EDIT_ROTATE_WEST))
        rotate.y -= rot_delta;
    // Rotate selected object east
    else if (chord_pressed(ACTION_EDIT_ROTATE_EAST))
        rotate.y += rot_delta;
    // Rotate selected object north
    else if (chord_pressed(ACTION_EDIT_ROTATE_NORTH))
        rotate.z -= rot_delta;
    // Rotate selected object south
    else if (chord_pressed(ACTION_EDIT_ROTATE_SOUTH))
        rotate.z += rot_delta;
    // Increase rotation delta
    else if (chord_pressed(ACTION_EDIT_ROTATE_DELTA_INCREASE))
    {
        if (rot_delta < ROT_DELTA_MAX)
        {
            rot_delta += (rot_delta < 5.0f) ? 1.0f : 5.0f;
            if (rot_delta > ROT_DELTA_MAX)
                rot_delta = ROT_DELTA_MAX;

            rot_delta_changed = true;
        }
    }
    // Decrease rotation delta
    else if (chord_pressed(ACTION_EDIT_ROTATE_DELTA_DECREASE))
    {
        if (rot_delta > ROT_DELTA_MIN)
        {
            rot_delta -= (rot_delta > 5.0f) ? 5.0f : 1.0f;
            if (rot_delta < ROT_DELTA_MIN)
                rot_delta = ROT_DELTA_MIN;

            rot_delta_changed = true;
        }
    }

    // Update selected object
    if (rotate_reset || !v3_equals(&rotate, &VEC3_ZERO))
    {
        selected_rotate(&rotate);
    }

    if (rot_delta_changed)
    {
        char buf[32] = { 0 };
        int len = snprintf(buf, sizeof(buf), "Rot Delta: %f", rot_delta);
        string_truncate(buf, sizeof(buf), len);

        string_free_slot(STR_SLOT_DELTA);
        struct rico_string *str;
        err = chunk_alloc((void **)&str, chunk_transient, RICO_HND_STRING);
        if (err) return err;
        err = string_init(str, rico_string_slot_string[STR_SLOT_DELTA],
                          STR_SLOT_DELTA, 0, 0, COLOR_DARK_BLUE_HIGHLIGHT,
                          1000, NULL, buf);
        if (err) return err;
    }

    return err;
}
internal int state_edit_scale()
{
    enum rico_error err = SUCCESS;

    err = shared_edit_events();   if (err || state != state_prev) return err;
    err = shared_engine_events(); if (err || state != state_prev) return err;
    err = shared_camera_events(); if (err || state != state_prev) return err;

    struct vec3 scale = VEC3_ZERO;
    bool scale_reset = false;
    bool scale_delta_changed = false;

    // Reset selected object's scale
    if (chord_pressed(ACTION_EDIT_SCALE_RESET))
    {
        scale_reset = true;
    }
    // Scale selected object up
    else if (chord_pressed(ACTION_EDIT_SCALE_UP))
    {
        scale.y += scale_delta;
    }
    // Scale selected object down
    else if (chord_pressed(ACTION_EDIT_SCALE_DOWN))
    {
        scale.y -= scale_delta;
    }
    // Scale selected object west
    else if (chord_pressed(ACTION_EDIT_SCALE_WEST))
    {
        scale.x -= scale_delta;
    }
    // Scale selected object east
    else if (chord_pressed(ACTION_EDIT_SCALE_EAST))
    {
        scale.x += scale_delta;
    }
    // Scale selected object north
    else if (chord_pressed(ACTION_EDIT_SCALE_NORTH))
    {
        scale.z += scale_delta;
    }
    // Scale selected object south
    else if (chord_pressed(ACTION_EDIT_SCALE_SOUTH))
    {
        scale.z -= scale_delta;
    }
    // Increase scale delta
    else if (chord_pressed(ACTION_EDIT_SCALE_DELTA_INCREASE))
    {
        if (scale_delta < SCALE_DELTA_MAX)
        {
            scale_delta += (scale_delta < 1.0f) ? 0.1f : 1.0f;
            if (scale_delta > SCALE_DELTA_MAX)
                scale_delta = SCALE_DELTA_MAX;

            scale_delta_changed = true;
        }
    }
    // Decrease scale delta
    else if (chord_pressed(ACTION_EDIT_SCALE_DELTA_DECREASE))
    {
        if (scale_delta > SCALE_DELTA_MIN)
        {
            scale_delta -= (scale_delta > 1.0f) ? 1.0f : 0.1f;
            if (scale_delta < SCALE_DELTA_MIN)
                scale_delta = SCALE_DELTA_MIN;

            scale_delta_changed = true;
        }
    }

    // Update selected object
    if (scale_reset || !v3_equals(&scale, &VEC3_ZERO))
    {
        selected_scale(&scale);
    }

    if (scale_delta_changed)
    {
        char buf[32] = { 0 };
        int len = snprintf(buf, sizeof(buf), "Scale Delta: %f", scale_delta);
        string_truncate(buf, sizeof(buf), len);

        string_free_slot(STR_SLOT_DELTA);
        struct rico_string *str;
        err = chunk_alloc((void **)&str, chunk_transient, RICO_HND_STRING);
        if (err) return err;
        err = string_init(str, rico_string_slot_string[STR_SLOT_DELTA],
                          STR_SLOT_DELTA, 0, 0, COLOR_DARK_BLUE_HIGHLIGHT,
                          1000, NULL, buf);
        if (err) return err;
    }

    return err;
}
internal int state_edit_texture()
{
    enum rico_error err = SUCCESS;

    err = shared_edit_events();   if (err || state != state_prev) return err;
    err = shared_engine_events(); if (err || state != state_prev) return err;
    err = shared_camera_events(); if (err || state != state_prev) return err;

    // TODO: Add texture events

    return err;
}
internal int state_edit_mesh()
{
    enum rico_error err = SUCCESS;

    err = shared_edit_events();   if (err || state != state_prev) return err;
    err = shared_engine_events(); if (err || state != state_prev) return err;
    err = shared_camera_events(); if (err || state != state_prev) return err;

    // Cycle selected object's mesh
    if (chord_pressed(ACTION_EDIT_MESH_NEXT))
    {
        selected_mesh_next();
    }
    // Cycle selected object's mesh (in reverse)
    else if (chord_pressed(ACTION_EDIT_MESH_PREVIOUS))
    {
        selected_mesh_prev();
    }
    // Recalculate bounding box based on current mesh
    else if (chord_pressed(ACTION_EDIT_MESH_BBOX_RECALCULATE))
    {
        selected_bbox_reset();
    }

    return err;
}
internal int state_menu_quit()
{
    enum rico_error err = SUCCESS;

    // [Y] / [Return]: Save and exit
    if (KEY_PRESSED(SDL_SCANCODE_Y) || KEY_PRESSED(SDL_SCANCODE_RETURN))
    {
        string_free_slot(STR_SLOT_MENU_QUIT);
        save_file();
        state = STATE_ENGINE_SHUTDOWN;
    }
    // [N] / [Escape]: Return to play mode
    else if (KEY_PRESSED(SDL_SCANCODE_N) || KEY_PRESSED(SDL_SCANCODE_ESCAPE))
    {
        string_free_slot(STR_SLOT_MENU_QUIT);
        state = STATE_PLAY_EXPLORE;
    }
    // [Q]: Exit without saving
    else if (KEY_PRESSED(SDL_SCANCODE_Q))
    {
        string_free_slot(STR_SLOT_MENU_QUIT);
        state = STATE_ENGINE_SHUTDOWN;
    }

    return err;
}
internal int state_text_input()
{
    enum rico_error err = SUCCESS;

    err = shared_edit_events();   if (err || state != state_prev) return err;
    err = shared_engine_events(); if (err || state != state_prev) return err;
    err = shared_camera_events(); if (err || state != state_prev) return err;

    // TODO: Handle text input

    return err;
}

internal int rico_init_shaders()
{
    enum rico_error err;

    // Create shader programs
    err = make_program_default(&prog_default);
    if (err) return err;

    err = make_program_primitive(&prog_primitive);
    return err;
}
internal void rico_init_cereal()
{
#if 0
    // Cleanup: Old serializiation methods
    // Custom serialiers
    rico_cereals[RICO_HND_CHUNK].save[0] = &chunk_serialize;
    rico_cereals[RICO_HND_CHUNK].load[0] = &chunk_deserialize;

    rico_cereals[RICO_HND_POOL].save[0] = &pool_serialize_0;
    rico_cereals[RICO_HND_POOL].load[0] = &pool_deserialize_0;

    rico_cereals[RICO_HND_OBJECT].save[0] = &object_serialize_0;
    rico_cereals[RICO_HND_OBJECT].load[0] = &object_deserialize_0;

    rico_cereals[RICO_HND_MATERIAL].save[0] = &material_serialize_0;
    rico_cereals[RICO_HND_MATERIAL].load[0] = &material_deserialize_0;

    rico_cereals[RICO_HND_BBOX].save[0] = &bbox_serialize_0;
    rico_cereals[RICO_HND_BBOX].load[0] = &bbox_deserialize_0;
#endif
}
internal int rico_init_transient_chunk()
{
    enum rico_error err;

    // TODO: Hard code defaults that make sense based on how many transient
    //       objects there actually are.
    const chunk_pool_counts pool_counts = {
          0,  // RICO_HND_NULL
        128,  // RICO_HND_OBJECT
        128,  // RICO_HND_TEXTURE
        128,  // RICO_HND_MESH
          4,  // RICO_HND_FONT
         32,  // RICO_HND_STRING
        128   // RICO_HND_MATERIAL
    };

    // Special chunk used to store transient object pools
    err = chunk_init(&chunk_transient, "[CHUNK_TRANSIENT]", &pool_counts);
    return err;
}
internal int init_hardcoded_test_chunk(struct rico_chunk **chunk)
{
    enum rico_error err = SUCCESS;
    UNUSED(chunk);

#if 0
    //--------------------------------------------------------------------------
    // Create chunk
    //--------------------------------------------------------------------------
    // TODO: Sane defaults, but use realloc when necessary
    const chunk_pool_counts pool_counts = {
          0,  // RICO_HND_NULL
        128,  // RICO_HND_OBJECT
        128,  // RICO_HND_TEXTURE
        128,  // RICO_HND_MESH
          4,  // RICO_HND_FONT
         32,  // RICO_HND_STRING
        128   // RICO_HND_MATERIAL
    };

    err = chunk_init(chunk, "[CHUNK_DEFAULT]", &pool_counts);
    if (err) return err;
    RICO_ASSERT(*chunk);

    //--------------------------------------------------------------------------
    // Create textures
    //--------------------------------------------------------------------------
    struct rico_texture *tex;

    /*
    "texture/grass.tga"
    "texture/hello.tga"
    "texture/fake_yellow.tga"
    */

    err = chunk_alloc((void **)&tex, *chunk, RICO_HND_TEXTURE);
    if (err) return err;
    err = texture_load_file(tex, "bricks", GL_TEXTURE_2D,
                            "texture/clean_bricks.tga", 32);

    //--------------------------------------------------------------------------
    // Create materials
    //--------------------------------------------------------------------------
    struct pool_id *tex_rock_id = hashtable_search_str(&global_textures,
                                                       "bricks");
    RICO_ASSERT(tex_rock_id->type);

    struct rico_material *material_rock;
    err = chunk_alloc((void **)&material_rock, *chunk, RICO_HND_MATERIAL);
    if (err) return err;
    err = material_init(material_rock, "Rock", *tex_rock_id, ID_NULL, 0.5f);
    if (err) return err;

    //--------------------------------------------------------------------------
    // Create world objects
    //--------------------------------------------------------------------------

    // Ground
    struct rico_object *obj_ground;
    err = chunk_alloc((void **)&obj_ground, *chunk, RICO_HND_OBJECT);
    if (err) return err;
    err = object_init(obj_ground, "Ground", OBJ_STATIC, ID_NULL,
                      material_rock->hnd.id, NULL);
    if (err) return err;
    object_rot_x(obj_ground, -90.0f);
    object_scale(obj_ground, &(struct vec3) { 64.0f, 64.0f, 0.001f });
    object_trans(obj_ground, &(struct vec3) { 0.0f, -1.0f, 0.0f });

#if 0
    //--------------------------------------------------------------------------
    // Save manual chunk
    //--------------------------------------------------------------------------
    struct rico_file file;
    err = rico_file_open_write(&file, "chunks/cereal.bin",
    RICO_FILE_VERSION_CURRENT);
    if (err) return err;

    err = rico_serialize(chunk_home, &file);
    rico_file_close(&file);
#endif
#endif

    return err;
}
internal int init_active_chunk()
{
    enum rico_error err;

    if (LOAD_SAVE_FILE)
    {
        struct rico_file file;
        err = rico_file_open_read(&file, "chunks/cereal.bin");
        if (err) return err;

        RICO_ASSERT(next_uid < UID_BASE);
        RICO_ASSERT(next_uid < file.next_uid);
        next_uid = file.next_uid;

        err = chunk_deserialize(&chunk_active, &file);
        rico_file_close(&file);
        return err;
    }
    else
    {
        RICO_ASSERT(next_uid < UID_BASE);
        next_uid = UID_BASE;

        printf("Loading hard-coded test chunk\n");
        err = init_hardcoded_test_chunk(&chunk_active);
        return err;
    }
}

internal int rico_init()
{
    enum rico_error err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing hash tables\n");
    printf("----------------------------------------------------------\n");
    rico_hashtable_init();

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing serializers\n");
    printf("----------------------------------------------------------\n");
    rico_init_cereal();

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing transient chunk\n");
    printf("----------------------------------------------------------\n");
    err = rico_init_transient_chunk();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing shaders\n");
    printf("----------------------------------------------------------\n");
    err = rico_init_shaders();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing primitives\n");
    printf("----------------------------------------------------------\n");
    prim_init(PRIM_SEGMENT);
    prim_init(PRIM_RAY);

#if 0
    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing fonts\n");
    printf("----------------------------------------------------------\n");
    err = rico_init_fonts();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing textures\n");
    printf("----------------------------------------------------------\n");
    err = rico_init_textures();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing materials\n");
    printf("----------------------------------------------------------\n");
    err = rico_init_materials();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing meshes\n");
    printf("----------------------------------------------------------\n");
    err = rico_init_meshes();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Loading meshes from resource files\n");
    printf("----------------------------------------------------------\n");
    err = load_mesh_files();
    if (err) return err;
#endif 

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Loading chunks\n");
    printf("----------------------------------------------------------\n");
    err = init_active_chunk();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing game world\n");
    printf("----------------------------------------------------------\n");
    init_glref();

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing camera\n");
    printf("----------------------------------------------------------\n");
    camera_reset(&cam_player);
    return err;
}
internal int state_engine_init()
{
    enum rico_error err;

    printf("==========================================================\n");
    printf("#        ______            _______        _              #\n");
    printf("#        |  __ \\ O        |__   __|      | |             #\n");
    printf("#        | |__| |_  ___ ___  | | ___  ___| |__           #\n");
    printf("#        |  _  /| |/ __/ _ \\ | |/ _ \\/ __| '_ \\          #\n");
    printf("#        | | \\ \\| | |_| (_) || |  __/ |__| | | |         #\n");
    printf("#        |_|  \\_\\_|\\___\\___/ |_|\\___|\\___|_| |_|         #\n");
    printf("#                                                        #\n");
    printf("#              Copyright 2017 Dan Bechard                #\n");
    printf("==========================================================\n");

    // TODO: Where does this belong? Needs access to "mouse_lock".
    SDL_SetRelativeMouseMode(mouse_lock);

    perfs_frequency = SDL_GetPerformanceFrequency();
    last_perfs = SDL_GetPerformanceCounter();
    last_cycles = __rdtsc();
    fps_last_render = last_perfs;
    view_vel = VEC3_ZERO;
    view_acc = VEC3_ZERO;

    // TODO: Support multiple SCANCODES at the same time (for e.g. CTRL, SHIFT)
    // TODO: Load from config file?
    // Initialize key map

    //struct rico_keychord blah = (struct rico_keychord) { 1, 2, 3 }

#define CHORD_3(action, k0, k1, k2) action_chords[action] = CHORD3(k0, k1, k2)
#define CHORD_2(action, k0, k1)     action_chords[action] = CHORD2(k0, k1)
#define CHORD_1(action, k0)         action_chords[action] = CHORD1(k0)

    // Engine
    CHORD_1(ACTION_ENGINE_DEBUG_LIGHTING_TOGGLE,    SDL_SCANCODE_L);
    CHORD_1(ACTION_ENGINE_DEBUG_TRIGGER_BREAKPOINT, SDL_SCANCODE_P);
    CHORD_1(ACTION_ENGINE_EDITOR_TOGGLE,            SDL_SCANCODE_GRAVE);
    CHORD_1(ACTION_ENGINE_FPS_TOGGLE,               SDL_SCANCODE_2);
    CHORD_1(ACTION_ENGINE_MOUSE_LOCK_TOGGLE,        SDL_SCANCODE_M);
    CHORD_1(ACTION_ENGINE_QUIT,                     SDL_SCANCODE_ESCAPE);

    // Camera
    CHORD_1(ACTION_CAMERA_SLOW_TOGGLE,              SDL_SCANCODE_R);
    CHORD_1(ACTION_CAMERA_RESET,                    SDL_SCANCODE_F);
    CHORD_1(ACTION_CAMERA_LOCK_TOGGLE,              SDL_SCANCODE_C);
    CHORD_1(ACTION_CAMERA_WIREFRAME_TOGGLE,         SDL_SCANCODE_1);

    // Player
    CHORD_1(ACTION_MOVE_RIGHT,                      SDL_SCANCODE_D);
    CHORD_1(ACTION_MOVE_LEFT,                       SDL_SCANCODE_A);
    CHORD_1(ACTION_MOVE_UP,                         SDL_SCANCODE_E);
    CHORD_1(ACTION_MOVE_DOWN,                       SDL_SCANCODE_Q);
    CHORD_1(ACTION_MOVE_FORWARD,                    SDL_SCANCODE_W);
    CHORD_1(ACTION_MOVE_BACKWARD,                   SDL_SCANCODE_S);
    CHORD_1(ACTION_MOVE_SPRINT,                     RICO_SCANCODE_SHIFT);

    // Editor
    CHORD_2(ACTION_EDIT_SAVE,                       RICO_SCANCODE_CTRL,   SDL_SCANCODE_S);
    CHORD_2(ACTION_EDIT_CYCLE_REVERSE,              RICO_SCANCODE_SHIFT,  SDL_SCANCODE_TAB);
    CHORD_1(ACTION_EDIT_CYCLE,                      SDL_SCANCODE_TAB);
    CHORD_2(ACTION_EDIT_BBOX_RECALCULATE,           RICO_SCANCODE_SHIFT,  SDL_SCANCODE_B);
    CHORD_1(ACTION_EDIT_CREATE_OBJECT,              SDL_SCANCODE_INSERT);
    CHORD_2(ACTION_EDIT_SELECTED_DUPLICATE,         RICO_SCANCODE_CTRL,   SDL_SCANCODE_D);
    CHORD_1(ACTION_EDIT_SELECTED_DELETE,            SDL_SCANCODE_DELETE);
    CHORD_1(ACTION_EDIT_MODE_PREVIOUS,              SDL_SCANCODE_KP_PERIOD);
    CHORD_1(ACTION_EDIT_MODE_NEXT,                  SDL_SCANCODE_KP_0);

    CHORD_1(ACTION_EDIT_TRANSLATE_RESET,            SDL_SCANCODE_0);
    CHORD_1(ACTION_EDIT_TRANSLATE_UP,               SDL_SCANCODE_UP);
    CHORD_1(ACTION_EDIT_TRANSLATE_DOWN,             SDL_SCANCODE_DOWN);
    CHORD_1(ACTION_EDIT_TRANSLATE_WEST,             SDL_SCANCODE_LEFT);
    CHORD_1(ACTION_EDIT_TRANSLATE_EAST,             SDL_SCANCODE_RIGHT);
    CHORD_1(ACTION_EDIT_TRANSLATE_NORTH,            SDL_SCANCODE_PAGEUP);
    CHORD_1(ACTION_EDIT_TRANSLATE_SOUTH,            SDL_SCANCODE_PAGEDOWN);
    CHORD_1(ACTION_EDIT_TRANSLATE_DELTA_INCREASE,   SDL_SCANCODE_KP_PLUS);
    CHORD_1(ACTION_EDIT_TRANSLATE_DELTA_DECREASE,   SDL_SCANCODE_KP_MINUS);

    CHORD_1(ACTION_EDIT_ROTATE_RESET,               SDL_SCANCODE_0);
    CHORD_1(ACTION_EDIT_ROTATE_UP,                  SDL_SCANCODE_UP);
    CHORD_1(ACTION_EDIT_ROTATE_DOWN,                SDL_SCANCODE_DOWN);
    CHORD_1(ACTION_EDIT_ROTATE_WEST,                SDL_SCANCODE_LEFT);
    CHORD_1(ACTION_EDIT_ROTATE_EAST,                SDL_SCANCODE_RIGHT);
    CHORD_1(ACTION_EDIT_ROTATE_NORTH,               SDL_SCANCODE_PAGEUP);
    CHORD_1(ACTION_EDIT_ROTATE_SOUTH,               SDL_SCANCODE_PAGEDOWN);
    CHORD_1(ACTION_EDIT_ROTATE_DELTA_INCREASE,      SDL_SCANCODE_KP_PLUS);
    CHORD_1(ACTION_EDIT_ROTATE_DELTA_DECREASE,      SDL_SCANCODE_KP_MINUS);
    CHORD_1(ACTION_EDIT_ROTATE_HEPTAMODE_TOGGLE,    SDL_SCANCODE_7);

    CHORD_1(ACTION_EDIT_SCALE_RESET,                SDL_SCANCODE_0);
    CHORD_1(ACTION_EDIT_SCALE_UP,                   SDL_SCANCODE_UP);
    CHORD_1(ACTION_EDIT_SCALE_DOWN,                 SDL_SCANCODE_DOWN);
    CHORD_1(ACTION_EDIT_SCALE_WEST,                 SDL_SCANCODE_LEFT);
    CHORD_1(ACTION_EDIT_SCALE_EAST,                 SDL_SCANCODE_RIGHT);
    CHORD_1(ACTION_EDIT_SCALE_NORTH,                SDL_SCANCODE_PAGEUP);
    CHORD_1(ACTION_EDIT_SCALE_SOUTH,                SDL_SCANCODE_PAGEDOWN);
    CHORD_1(ACTION_EDIT_SCALE_DELTA_INCREASE,       SDL_SCANCODE_KP_PLUS);
    CHORD_1(ACTION_EDIT_SCALE_DELTA_DECREASE,       SDL_SCANCODE_KP_MINUS);

    CHORD_1(ACTION_EDIT_MESH_NEXT,                  SDL_SCANCODE_RIGHT);
    CHORD_1(ACTION_EDIT_MESH_PREVIOUS,              SDL_SCANCODE_LEFT);
    CHORD_1(ACTION_EDIT_MESH_BBOX_RECALCULATE,      SDL_SCANCODE_B);

    err = rico_init();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][  GO] Initialization complete. Starting game.\n");
    printf("----------------------------------------------------------\n");

    state = STATE_PLAY_EXPLORE;
    return err;
}
internal int state_engine_shutdown()
{
    free_glref();
    return SUCCESS;
}
void init_rico_engine()
{
    state_handlers[STATE_ENGINE_INIT    ].run = &state_engine_init;
    state_handlers[STATE_ENGINE_SHUTDOWN].run = &state_engine_shutdown;
    state_handlers[STATE_MENU_QUIT      ].run = &state_menu_quit;
    state_handlers[STATE_PLAY_EXPLORE   ].run = &state_play_explore;
    state_handlers[STATE_EDIT_TRANSLATE ].run = &state_edit_translate;
    state_handlers[STATE_EDIT_ROTATE    ].run = &state_edit_rotate;
    state_handlers[STATE_EDIT_SCALE     ].run = &state_edit_scale;
    state_handlers[STATE_EDIT_TEXTURE   ].run = &state_edit_texture;
    state_handlers[STATE_EDIT_MESH      ].run = &state_edit_mesh;
    state_handlers[STATE_TEXT_INPUT     ].run = &state_text_input;

    state_handlers[STATE_EDIT_TRANSLATE ].cleanup = &state_edit_cleanup;
    state_handlers[STATE_EDIT_ROTATE    ].cleanup = &state_edit_cleanup;
    state_handlers[STATE_EDIT_SCALE     ].cleanup = &state_edit_cleanup;
    state_handlers[STATE_EDIT_TEXTURE   ].cleanup = &state_edit_cleanup;
    state_handlers[STATE_EDIT_MESH      ].cleanup = &state_edit_cleanup;

    state = STATE_ENGINE_INIT;
}
