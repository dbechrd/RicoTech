#define LOAD_SAVE_FILE false

#include "rico_input.h"

///|////////////////////////////////////////////////////////////////////////////
const char *rico_state_string[] = {
    RICO_STATES(GEN_STRING)
};

static struct rico_keychord action_chords[ACTION_COUNT] = { 0 };

struct pack *pack_active = 0;

///|////////////////////////////////////////////////////////////////////////////
//Human walk speed empirically found to be 33 steps in 20 seconds. That is
// approximately 1.65 steps per second. At 60 fps, that is 0.0275 steps per
// frame. Typical walking stride is ~0.762 meters (30 inches). Distance traveled
// per frame (60hz) is 0.762 * 0.0275 = 0.020955 ~= 0.021

r32 mouse_dx = 0;
r32 mouse_dy = 0;

internal struct vec3 player_acc;

#define CAM_SLOW_MULTIPLIER 0.1f
#define CAM_SPRINT_MULTIPLIER 5.0f
internal bool player_sprint = false;
internal bool camera_slow = false;

///|////////////////////////////////////////////////////////////////////////////
#define TRANS_DELTA_MIN 0.01f
#define TRANS_DELTA_MAX 10.0f
float trans_delta = 0.1f;

#define ROT_DELTA_MIN 1.0f
#define ROT_DELTA_MAX 90.0f
#define ROT_DELTA_DEFAULT 5.0f
internal float rot_delta = ROT_DELTA_DEFAULT;

#define SCALE_DELTA_MIN 0.1f
#define SCALE_DELTA_MAX 5.0f
internal float scale_delta = 1.0f;

internal bool mouse_lock = true;
internal bool enable_lighting = true;
internal bool audio_muted = false;

///|////////////////////////////////////////////////////////////////////////////
// Simulation params
internal r64 sim_accum = 0;

// Performance timing
internal u64 perfs_frequency;
internal u64 last_perfs;
internal u64 last_cycles;

// FPS UI
internal u64 fps_last_render;
internal u64 fps_render_delta = 200000;  // 200 ms
internal bool fps_render = false;
internal bool vsync = true;

///|////////////////////////////////////////////////////////////////////////////
// Mouse and keyboard state
internal u32 mouse_buttons = 0;
internal u32 mouse_buttons_prev = 0;

internal u8 keystate_buffers[2][SDL_NUM_SCANCODES] = { 0 };
internal u8 *keys      = keystate_buffers[0];
internal u8 *keys_prev = keystate_buffers[1];

const rico_key RICO_SCANCODE_ALT   = (rico_key)301;
const rico_key RICO_SCANCODE_CTRL  = (rico_key)302;
const rico_key RICO_SCANCODE_SHIFT = (rico_key)303;
const rico_key RICO_SCANCODE_LMB   = (rico_key)304;
const rico_key RICO_SCANCODE_MMB   = (rico_key)305;
const rico_key RICO_SCANCODE_RMB   = (rico_key)306;

#define KEY_IS_DOWN(key) (key == 0 || keys[key] || \
(key == RICO_SCANCODE_ALT   && (keys[SDL_SCANCODE_LALT]   || keys[SDL_SCANCODE_RALT]))  || \
(key == RICO_SCANCODE_CTRL  && (keys[SDL_SCANCODE_LCTRL]  || keys[SDL_SCANCODE_RCTRL])) || \
(key == RICO_SCANCODE_SHIFT && (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])) || \
(key == RICO_SCANCODE_LMB && (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT))) || \
(key == RICO_SCANCODE_MMB && (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE))) || \
(key == RICO_SCANCODE_RMB && (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT))))

#define KEY_WAS_DOWN(key) (key == 0 || keys_prev[key] || \
(key == RICO_SCANCODE_ALT   && (keys_prev[SDL_SCANCODE_LALT]   || keys_prev[SDL_SCANCODE_RALT]))  || \
(key == RICO_SCANCODE_CTRL  && (keys_prev[SDL_SCANCODE_LCTRL]  || keys_prev[SDL_SCANCODE_RCTRL])) || \
(key == RICO_SCANCODE_SHIFT && (keys_prev[SDL_SCANCODE_LSHIFT] || keys_prev[SDL_SCANCODE_RSHIFT])) || \
(key == RICO_SCANCODE_LMB && (mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_LEFT))) || \
(key == RICO_SCANCODE_MMB && (mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_MIDDLE))) || \
(key == RICO_SCANCODE_RMB && (mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_RIGHT))))

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

// Current state
internal enum rico_state state_prev;
internal enum rico_state state;

// State change handlers
typedef int (*state_handler)();
struct state_handlers
{
    state_handler init;
    state_handler run;
    state_handler cleanup;
};
struct state_handlers state_handlers[STATE_COUNT] = { 0 };

inline enum rico_state state_get()
{
    return state;
}
inline bool state_is_edit()
{
    return (state == STATE_EDIT_TRANSLATE ||
            state == STATE_EDIT_ROTATE ||
            state == STATE_EDIT_SCALE ||
            state == STATE_EDIT_MATERIAL ||
            state == STATE_EDIT_MESH);
}
inline bool state_is_paused()
{
    return (state == STATE_MENU_QUIT);
}

void render_fps(r64 fps, r64 ms, r64 mcyc)
{
    // TODO: Allow x/y coords to be given in characters instead of pixels
    //       based on FONT_WIDTH / FONT_HEIGHT.
    char buf[128] = { 0 };
    int len = snprintf(buf, sizeof(buf), "%.f fps %.2f ms %.2f mcyc", fps, ms,
                       mcyc);
    string_truncate(buf, sizeof(buf), len);
    string_free_slot(STR_SLOT_FPS);
    load_string(packs[PACK_TRANSIENT], rico_string_slot_string[STR_SLOT_FPS],
                STR_SLOT_FPS, SCREEN_X(-(FONT_WIDTH * len)), SCREEN_Y(0),
                COLOR_DARK_RED_HIGHLIGHT, 0, NULL, buf);
}

int state_update()
{
    enum rico_error err;

    ///-------------------------------------------------------------------------
    //| Clear previous frame
    ///-------------------------------------------------------------------------
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    //glClearColor(0.46f, 0.70f, 1.0f, 1.0f);
    //glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ///-------------------------------------------------------------------------
    //| Query input state
    ///-------------------------------------------------------------------------
    // Get mouse state
    int dx, dy;
    mouse_buttons_prev = mouse_buttons;
    mouse_buttons = SDL_GetRelativeMouseState(&dx, &dy);
    if (mouse_lock)
    {
        mouse_dx += (r32)dx;
        mouse_dy += (r32)dy;
    }
    else
    {
        mouse_dx = mouse_dy = 0;
    }

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
        load_string(packs[PACK_TRANSIENT],
                    rico_string_slot_string[STR_SLOT_STATE], STR_SLOT_STATE,
                    SCREEN_X(0), SCREEN_Y(0), COLOR_DARK_RED_HIGHLIGHT, 0, NULL,
                    buf);
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

    r64 frame_dt = (r64)delta_perfs / perfs_frequency;
    r64 frame_ms = frame_dt * 1000.0;

    // Update FPS counter string if enabled and overdue
    if (fps_render && last_perfs - fps_last_render > fps_render_delta)
    {
        r64 fps = (r64)perfs_frequency / delta_perfs;
        r64 mcyc = (r64)delta_cycles / (1000.0 * 1000.0); // MegaCycles/Frame

        render_fps(fps, frame_ms, mcyc);
        fps_last_render = last_perfs;
    }

    ///-------------------------------------------------------------------------
    //| Update
    ///-------------------------------------------------------------------------
    sim_accum += MIN(frame_ms, SIM_MAX_FRAMESKIP_MS);
    while (sim_accum >= SIM_MS)
    {
        if (!state_is_paused(state))
        {
            struct vec3 camera_acc = player_acc;
            if (player_sprint) v3_scalef(&camera_acc, CAM_SPRINT_MULTIPLIER);
            if (camera_slow)   v3_scalef(&camera_acc, CAM_SLOW_MULTIPLIER);
            player_camera_update(&cam_player, mouse_dx, mouse_dy, camera_acc);

            // TODO: Move this to this program.c
            // Update uniforms
            RICO_ASSERT(prog_pbr->prog_id);
            glUseProgram(prog_pbr->prog_id);
            glUniform1f(prog_pbr->time, (r32)SIM_SEC);
            glUseProgram(0);
        }

        string_update();

        mouse_dx = 0;
        mouse_dy = 0;
        sim_accum -= SIM_MS;
    }

    ///-------------------------------------------------------------------------
    //| Render
    ///-------------------------------------------------------------------------
    r64 alpha = (r64)sim_accum / SIM_MS;

    // glPolygonMode(GL_FRONT_AND_BACK, cam_player.fill_mode);
    glPolygonMode(GL_FRONT, cam_player.fill_mode);

    // TODO: Use alpha
    // glref_render(alpha, &cam_player);
    // camera_render(alpha, &cam_player);
    UNUSED(alpha);
    glref_render(&cam_player);
    camera_render(&cam_player);

    return err;
}

internal int shared_engine_events()
{
    enum rico_error err = SUCCESS;

    // Toggle scene lighting
    if (chord_pressed(ACTION_ENGINE_DEBUG_LIGHTING_TOGGLE))
    {
        // TODO: Pretty sure this is broken
        // TODO: Use this to change shader program on render
        enable_lighting = !enable_lighting;
    }
#if RICO_DEBUG
    // DEBUG: Trigger breakpoint
    else if (chord_pressed(ACTION_ENGINE_DEBUG_TRIGGER_BREAKPOINT))
    {
        SDL_TriggerBreakpoint();
    }
#endif
    // Toggle FPS counter
    else if (chord_pressed(ACTION_ENGINE_FPS_TOGGLE))
    {
        fps_render = !fps_render;
        if (!fps_render)
        {
            string_free_slot(STR_SLOT_FPS);
        }
    }
    // Toggle mouse lock-to-window
    else if (chord_pressed(ACTION_ENGINE_MOUSE_LOCK_TOGGLE))
    {
        mouse_lock = !mouse_lock;
        SDL_SetRelativeMouseMode(mouse_lock);
    }
    // Toggle vsync
    else if (chord_pressed(ACTION_ENGINE_VSYNC_TOGGLE))
    {
        vsync = !vsync;
        SDL_GL_SetSwapInterval(vsync);
    }
    // Toggle audio
    else if (chord_pressed(ACTION_ENGINE_MUTE_TOGGLE))
    {
        audio_muted = !audio_muted;
        alSourcef(audio_source, AL_GAIN, (audio_muted) ? 0.0f : 1.0f);

        char buf[16] = { 0 };
        int len;
        if (audio_muted)
        {
            len = snprintf(buf, sizeof(buf), "Volume:   0%%");
        }
        else
        {
            len = snprintf(buf, sizeof(buf), "Volume: 100%%");
        }
        string_truncate(buf, sizeof(buf), len);
        string_free_slot(STR_SLOT_DEBUG);
        load_string(packs[PACK_TRANSIENT],
                    rico_string_slot_string[STR_SLOT_DEBUG], STR_SLOT_DEBUG,
                    SCREEN_X(-150), SCREEN_Y(0), COLOR_DARK_GRAY, 1000, NULL,
                    buf);
    }
    // Save and exit
    else if (chord_pressed(ACTION_ENGINE_QUIT))
    {
        string_free_slot(STR_SLOT_MENU_QUIT);
        load_string(packs[PACK_TRANSIENT],
                    rico_string_slot_string[STR_SLOT_MENU_QUIT],
                    STR_SLOT_MENU_QUIT, SCREEN_X(SCREEN_W / 2 - 92),
                    SCREEN_Y(SCREEN_H / 2 - 128),
                    COLOR_DARK_GREEN_HIGHLIGHT, 0, NULL,
                    "                       \n" \
                    "  Save and quit?       \n" \
                    "                       \n" \
                    "  [Y] Yes              \n" \
                    "  [N] No               \n" \
                    "  [Q] Quit w/o saving  \n" \
                    "                       ");
        state = STATE_MENU_QUIT;
    }

    return err;
}
internal int shared_camera_events()
{
    enum rico_error err = SUCCESS;

    player_acc = VEC3_ZERO;
    if (chord_is_down(ACTION_MOVE_UP))       player_acc.y += 1.0f;
    if (chord_is_down(ACTION_MOVE_DOWN))     player_acc.y -= 1.0f;
    if (chord_is_down(ACTION_MOVE_RIGHT))    player_acc.x += 1.0f;
    if (chord_is_down(ACTION_MOVE_LEFT))     player_acc.x -= 1.0f;
    if (chord_is_down(ACTION_MOVE_FORWARD))  player_acc.z += 1.0f;
    if (chord_is_down(ACTION_MOVE_BACKWARD)) player_acc.z -= 1.0f;
    v3_normalize(&player_acc);  // NOTE: Prevent faster movement on diagonal

    player_sprint = chord_is_down(ACTION_MOVE_SPRINT);

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
    RICO_ASSERT(state_is_edit(state));

    enum rico_error err = SUCCESS;

    // Raycast object selection
    if (chord_pressed(ACTION_EDIT_MOUSE_PICK))
    {
        edit_mouse_pressed();
    }
    else if (chord_is_down(ACTION_EDIT_MOUSE_PICK))
    {
        if (mouse_dx || mouse_dy)
        {
            edit_mouse_move();
        }
    }
    else if (chord_released(ACTION_EDIT_MOUSE_PICK))
    {
        edit_mouse_released();
    }
    // Recalculate bounding boxes of all objects
    else if (chord_pressed(ACTION_EDIT_BBOX_RECALCULATE))
    {
        edit_bbox_reset_all();
    }
    // Duplicate selected object
    else if (chord_pressed(ACTION_EDIT_SELECTED_DUPLICATE))
    {
        edit_duplicate();
    }
    // Exit edit mode
    else if (chord_pressed(ACTION_EDIT_QUIT))
    {
		edit_object_select(0, false);
        state = STATE_PLAY_EXPLORE;
    }
    // Select previous edit mode
    else if (chord_pressed(ACTION_EDIT_MODE_PREVIOUS))
    {
        switch (state)
        {
        case STATE_EDIT_ROTATE:
        case STATE_EDIT_SCALE:
        case STATE_EDIT_MATERIAL:
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
        case STATE_EDIT_MATERIAL:
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
        edit_object_prev();
    }
    // Cycle select through objects
    else if (chord_pressed(ACTION_EDIT_CYCLE))
    {
        edit_object_next();
    }
    // Create new object
    else if (chord_pressed(ACTION_EDIT_CREATE_OBJECT))
    {
        edit_object_create(pack_active);
    }
    // Delete selected object
    else if (chord_pressed(ACTION_EDIT_SELECTED_DELETE))
    {
        edit_delete();
    }
    // Save chunk
    else if (chord_pressed(ACTION_EDIT_SAVE))
    {
		err = pack_save(pack_active, pack_active->name, false);
    }

    return err;
}

internal int state_play_explore()
{
    enum rico_error err = SUCCESS;

    // Check global engine / editor chords after the more specific checks run
    // to allow overriding the global keybinds while in more specific states.
    // This is like doing base.Foo() at the end of an overridden Foo() method.
    // It might also be useful to have a stack of states, e.g.:
    // active_states = { ENGINE, EDITOR, TEXTURE }
    // active_states = { ENGINE, GAME, FLY }
    // Alternatively, stack can be used for "pushdown automata", where you
    // need to pop to a previous state when the current one finishes. e.g.:
    // state_stack.push(WALK);
    // state_stack.push(JUMP);
    // state_stack.pop(JUMP);
    err = shared_engine_events(); if (err || state != state_prev) return err;
    err = shared_camera_events(); if (err || state != state_prev) return err;

    // Interact with clicked object
    if (chord_pressed(ACTION_PLAY_INTERACT))
    {
        struct rico_object *obj = mouse_first_obj();
        if (obj)
            object_interact(obj);
    }
    // Enter edit mode
    else if (chord_pressed(ACTION_PLAY_EDITOR))
    {
        state = STATE_EDIT_TRANSLATE;
        edit_print_object();
    }

    return err;
}
internal int state_edit_cleanup()
{
    enum rico_error err = SUCCESS;

    if (!state_is_edit(state))
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
        edit_translate(&cam_player, &translate);
    }

    if (trans_delta_changed)
    {
        char buf[32] = { 0 };
        int len = snprintf(buf, sizeof(buf), "Trans Delta: %f", trans_delta);
        string_truncate(buf, sizeof(buf), len);
        string_free_slot(STR_SLOT_DELTA);
        load_string(packs[PACK_TRANSIENT],
                    rico_string_slot_string[STR_SLOT_DELTA], STR_SLOT_DELTA,
                    SCREEN_X(0), SCREEN_Y(0), COLOR_DARK_BLUE_HIGHLIGHT, 1000,
                    NULL, buf);
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
        edit_rotate(&rotate);
    }

    if (rot_delta_changed)
    {
        char buf[32] = { 0 };
        int len = snprintf(buf, sizeof(buf), "Rot Delta: %f", rot_delta);
        string_truncate(buf, sizeof(buf), len);

        string_free_slot(STR_SLOT_DELTA);
        load_string(packs[PACK_TRANSIENT],
                    rico_string_slot_string[STR_SLOT_DELTA], STR_SLOT_DELTA,
                    SCREEN_X(0), SCREEN_Y(0), COLOR_DARK_BLUE_HIGHLIGHT, 1000,
                    NULL, buf);
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
        edit_scale(&scale);
    }

    if (scale_delta_changed)
    {
        char buf[32] = { 0 };
        int len = snprintf(buf, sizeof(buf), "Scale Delta: %f", scale_delta);
        string_truncate(buf, sizeof(buf), len);
        string_free_slot(STR_SLOT_DELTA);
        load_string(packs[PACK_TRANSIENT],
                    rico_string_slot_string[STR_SLOT_DELTA], STR_SLOT_DELTA,
                    SCREEN_X(0), SCREEN_Y(0), COLOR_DARK_BLUE_HIGHLIGHT, 1000,
                    NULL, buf);
    }

    return err;
}
internal int state_edit_material()
{
    enum rico_error err = SUCCESS;

    err = shared_edit_events();   if (err || state != state_prev) return err;
    err = shared_engine_events(); if (err || state != state_prev) return err;
    err = shared_camera_events(); if (err || state != state_prev) return err;

    // Cycle selected object's material
    if (chord_pressed(ACTION_EDIT_MATERIAL_NEXT))
    {
        edit_material_next();
    }
    // Cycle selected object's material (in reverse)
    else if (chord_pressed(ACTION_EDIT_MATERIAL_PREVIOUS))
    {
        edit_material_prev();
    }

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
        edit_mesh_next();
    }
    // Cycle selected object's mesh (in reverse)
    else if (chord_pressed(ACTION_EDIT_MESH_PREVIOUS))
    {
        edit_mesh_prev();
    }
    // Recalculate bounding box based on current mesh
    else if (chord_pressed(ACTION_EDIT_MESH_BBOX_RECALCULATE))
    {
        edit_bbox_reset();
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
		err = pack_save(pack_active, pack_active->name, false);
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
    err = make_program_pbr(&prog_pbr);
    if (err) return err;

    err = make_program_primitive(&prog_prim);
    if (err) return err;

    err = make_program_text(&prog_text);
    if (err) return err;

    return err;
}
#if 0
internal void rico_init_cereal()
{
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
    object_scale(obj_ground, &VEC3(64.0f, 64.0f, 0.001f));
    object_trans(obj_ground, &VEC3(0.0f, -1.0f, 0.0f));

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
#endif

internal int rico_init()
{
    enum rico_error err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing hash tables\n");
    printf("----------------------------------------------------------\n");
    rico_hashtable_init();

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing shaders\n");
    printf("----------------------------------------------------------\n");
    err = rico_init_shaders();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing primitives\n");
    printf("----------------------------------------------------------\n");
    prim_init();

#if 0
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

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Loading chunks\n");
    printf("----------------------------------------------------------\n");
    err = init_active_chunk();
    if (err) return err;
#endif

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing editor\n");
    printf("----------------------------------------------------------\n");
    editor_init();

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing packs\n");
    printf("----------------------------------------------------------\n");
    err = pack_load_all();
    if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing camera\n");
    printf("----------------------------------------------------------\n");
    camera_reset(&cam_player);
    if (err) return err;

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

    perfs_frequency = SDL_GetPerformanceFrequency();
    last_perfs = SDL_GetPerformanceCounter();
    last_cycles = __rdtsc();
    fps_last_render = last_perfs;

    int sdl_err = SDL_GL_SetSwapInterval(vsync);
    if (sdl_err < 0) {
        return RICO_ERROR(ERR_SDL_INIT, "SDL_GL_SetSwapInterval error: %s",
                          SDL_GetError());
    }

    // Reset mouse delta after window opens and mouse is locked to screen
    SDL_SetRelativeMouseMode(mouse_lock);
    mouse_dx = 0;
    mouse_dy = 0;

    // TODO: Load from config file?
    // Initialize key map

#define CHORD_3(action, k0, k1, k2) action_chords[action] = CHORD3(k0, k1, k2)
#define CHORD_2(action, k0, k1)     action_chords[action] = CHORD2(k0, k1)
#define CHORD_1(action, k0)         action_chords[action] = CHORD1(k0)

    // Engine
    CHORD_1(ACTION_ENGINE_DEBUG_LIGHTING_TOGGLE,    SDL_SCANCODE_L);
    CHORD_1(ACTION_ENGINE_DEBUG_TRIGGER_BREAKPOINT, SDL_SCANCODE_P);
    CHORD_1(ACTION_ENGINE_FPS_TOGGLE,               SDL_SCANCODE_2);
    CHORD_1(ACTION_ENGINE_MOUSE_LOCK_TOGGLE,        SDL_SCANCODE_M);
    CHORD_1(ACTION_ENGINE_VSYNC_TOGGLE,             SDL_SCANCODE_V);
    CHORD_1(ACTION_ENGINE_MUTE_TOGGLE,              SDL_SCANCODE_PERIOD);
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

    CHORD_1(ACTION_PLAY_EDITOR,                     SDL_SCANCODE_GRAVE);
    CHORD_1(ACTION_PLAY_INTERACT,                   RICO_SCANCODE_LMB);

    // Editor
    CHORD_1(ACTION_EDIT_QUIT,                       SDL_SCANCODE_GRAVE);
    CHORD_2(ACTION_EDIT_SAVE,                       RICO_SCANCODE_CTRL,   SDL_SCANCODE_S);
    CHORD_2(ACTION_EDIT_CYCLE_REVERSE,              RICO_SCANCODE_SHIFT,  SDL_SCANCODE_TAB);
    CHORD_1(ACTION_EDIT_CYCLE,                      SDL_SCANCODE_TAB);
    CHORD_1(ACTION_EDIT_MOUSE_PICK,                 RICO_SCANCODE_LMB);
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

    CHORD_1(ACTION_EDIT_MATERIAL_NEXT,              SDL_SCANCODE_RIGHT);
    CHORD_1(ACTION_EDIT_MATERIAL_PREVIOUS,          SDL_SCANCODE_LEFT);

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
    state_handlers[STATE_EDIT_MATERIAL  ].run = &state_edit_material;
    state_handlers[STATE_EDIT_MESH      ].run = &state_edit_mesh;
    state_handlers[STATE_TEXT_INPUT     ].run = &state_text_input;

    state_handlers[STATE_EDIT_TRANSLATE ].cleanup = &state_edit_cleanup;
    state_handlers[STATE_EDIT_ROTATE    ].cleanup = &state_edit_cleanup;
    state_handlers[STATE_EDIT_SCALE     ].cleanup = &state_edit_cleanup;
    state_handlers[STATE_EDIT_MATERIAL  ].cleanup = &state_edit_cleanup;
    state_handlers[STATE_EDIT_MESH      ].cleanup = &state_edit_cleanup;

    state = STATE_ENGINE_INIT;
}
