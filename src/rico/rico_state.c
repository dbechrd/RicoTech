#define LOAD_SAVE_FILE false

#define MAX_ACTIONS 256
#define MAX_ACTIONS_QUEUED 32

// u32 action -> keychord
static struct ric_keychord action_chords[MAX_ACTIONS] = { 0 };
static u32 action_max = 0;
// int index -> u32 action
static u32 action_queue[MAX_ACTIONS_QUEUED] = { 0 };
static u32 action_queue_count = 0;

// Human walk speed empirically found to be 33 steps in 20 seconds. That is
// approximately 1.65 steps per second. At 60 fps, that is 0.0275 steps per
// frame. Typical walking stride is ~0.762 meters (30 inches). Distance traveled
// per frame (60hz) is 0.762 * 0.0275 = 0.020955 ~= 0.021

static s32 mouse_x;
static s32 mouse_y;
static s32 mouse_dx = 0;
static s32 mouse_dy = 0;

static struct vec3 player_acc;

#define CAM_SLOW_MULTIPLIER 0.1f
#define CAM_SPRINT_MULTIPLIER 5.0f
static bool player_sprint = false;
static bool camera_slow = false;

#define TRANS_DELTA_MIN 0.01f
#define TRANS_DELTA_MAX 10.0f
float global_trans_delta = 0.1f;

#define ROT_DELTA_MIN 1.0f
#define ROT_DELTA_MAX 90.0f
#define ROT_DELTA_DEFAULT 5.0f
float global_rot_delta = ROT_DELTA_DEFAULT;

#define SCALE_DELTA_MIN 0.1f
#define SCALE_DELTA_MAX 5.0f
float global_scale_delta = 1.0f;

static bool mouse_lock = true;
static bool enable_lighting = true;

// Simulation params
static r64 sim_accum = 0;
static r64 sim_alpha = 0;
static bool sim_paused_prev = false;
static bool sim_paused = false;

// Performance timing
static u64 perfs_frequency;
static u64 init_perfs;
static u64 last_perfs;
static u64 last_cycles;

// FPS UI
static u64 fps_last_render;
static u64 fps_render_delta = 200000;  // 200 ms
static bool fps_render = false;
static bool vsync = true;

// Mouse and keyboard state
static u32 mouse_buttons = 0;
static u32 mouse_buttons_prev = 0;

static u8 keystate_buffers[2][SDL_NUM_SCANCODES] = { 0 };
static u8 *keys      = keystate_buffers[0];
static u8 *keys_prev = keystate_buffers[1];

#define KEY_IS_DOWN(key) (key == 0 || keys[key] || \
(key == RIC_SCANCODE_ALT   && (keys[SDL_SCANCODE_LALT]   || keys[SDL_SCANCODE_RALT]))  || \
(key == RIC_SCANCODE_CTRL  && (keys[SDL_SCANCODE_LCTRL]  || keys[SDL_SCANCODE_RCTRL])) || \
(key == RIC_SCANCODE_SHIFT && (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])) || \
(key == RIC_SCANCODE_LMB && (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT))) || \
(key == RIC_SCANCODE_MMB && (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE))) || \
(key == RIC_SCANCODE_RMB && (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT))))

#define KEY_WAS_DOWN(key) (key == 0 || keys_prev[key] || \
(key == RIC_SCANCODE_ALT   && (keys_prev[SDL_SCANCODE_LALT]   || keys_prev[SDL_SCANCODE_RALT]))  || \
(key == RIC_SCANCODE_CTRL  && (keys_prev[SDL_SCANCODE_LCTRL]  || keys_prev[SDL_SCANCODE_RCTRL])) || \
(key == RIC_SCANCODE_SHIFT && (keys_prev[SDL_SCANCODE_LSHIFT] || keys_prev[SDL_SCANCODE_RSHIFT])) || \
(key == RIC_SCANCODE_LMB && (mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_LEFT))) || \
(key == RIC_SCANCODE_MMB && (mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_MIDDLE))) || \
(key == RIC_SCANCODE_RMB && (mouse_buttons_prev & SDL_BUTTON(SDL_BUTTON_RIGHT))))

#define KEY_PRESSED(key)  ( KEY_IS_DOWN(key) && !KEY_WAS_DOWN(key))
#define KEY_RELEASED(key) (!KEY_IS_DOWN(key) &&  KEY_WAS_DOWN(key))

#define CHORD_VALID(chord) (chord->keys[0] || chord->keys[1] || chord->keys[2])

static inline bool chord_is_down(const struct ric_keychord *chord)
{
    // All keys are down
    return (CHORD_VALID(chord) &&
            KEY_IS_DOWN(chord->keys[0]) &&
            KEY_IS_DOWN(chord->keys[1]) &&
            KEY_IS_DOWN(chord->keys[2]));
}
static inline bool chord_was_down(const struct ric_keychord *chord)
{
    // All keys were down last frame
    return (CHORD_VALID(chord) &&
            KEY_WAS_DOWN(chord->keys[0]) &&
            KEY_WAS_DOWN(chord->keys[1]) &&
            KEY_WAS_DOWN(chord->keys[2]));
}
#if 0
// Cleanup, using keychord flags for this now
static inline bool chord_pressed(u32 action)
{
    // All keys are down, and at least one was up last frame
    return chord_is_down(action) && !chord_was_down(action);
}
static inline bool chord_released(u32 action)
{
    // At least one key is up, and all were down last frame
    return !chord_is_down(action) && chord_was_down(action);
}
#endif
static inline bool chord_active(const struct ric_keychord *chord)
{
    // Chord is considered active when any of the following is true:
    // 1) is down and repeat
    // 2) pressed (is down and wasn't down last frame)
    // 3) on_release and released (is not down but was last frame).
    bool active = false;
    if (chord_is_down(chord))
    {
        active = chord->repeat || !chord_was_down(chord);
    }
    else
    {
        active = chord->on_release && chord_was_down(chord);
    }
    return active;
}
static inline bool action_active(u32 action)
{
    return chord_active(&action_chords[action]);
}

// Current state
static enum ric_state state_prev;
static enum ric_state state_last_frame;
static enum ric_state state;

// State change handlers
typedef int (*state_handler)();
struct state_handlers
{
    state_handler init;
    state_handler run;
    state_handler cleanup;
};
static struct state_handlers state_handlers[RIC_ENGINE_STATE_COUNT] = { 0 };

extern r64 ric_simulation_time()
{
    r64 now = (r64)SDL_GetPerformanceCounter();
    r64 time = (now - init_perfs) / perfs_frequency * 1000.0;
    return time;
}
extern bool ric_simulation_paused()
{
    return sim_paused;
}
extern void ric_simulation_pause()
{
    sim_paused_prev = sim_paused;
    sim_paused = true;
}
extern void ric_simulation_play()
{
    sim_paused_prev = sim_paused;
    sim_paused = false;
}
extern bool ric_simulation_prev()
{
    bool prev = sim_paused_prev;
    sim_paused_prev = sim_paused;
    sim_paused = prev;
    return sim_paused;
}
extern enum ric_state ric_state()
{
    return state;
}
extern bool ric_state_is_menu()
{
    return (state == RIC_ENGINE_MENU_QUIT ||
            state == RIC_ENGINE_TEXT_INPUT);
}
extern bool ric_state_is_edit()
{
    return (state == RIC_ENGINE_EDIT_TRANSLATE ||
            state == RIC_ENGINE_EDIT_ROTATE ||
            state == RIC_ENGINE_EDIT_SCALE ||
            state == RIC_ENGINE_EDIT_MATERIAL ||
            state == RIC_ENGINE_EDIT_MESH);
}

static void render_fps(r64 fps, r64 ms, r64 mcyc)
{
    char buf[128] = { 0 };
    int len = snprintf(buf, sizeof(buf), "%.f fps %.2f ms %.2f mcyc", fps, ms,
                       mcyc);
    string_truncate(buf, sizeof(buf), len);
    string_free_slot(RIC_STRING_SLOT_FPS);

    // TODO: Allocate in frame arena, not transient
    ric_load_string(RIC_PACK_ID_TRANSIENT, RIC_STRING_SLOT_FPS,
                     SCREEN_X(-(FONT_WIDTH * len)), SCREEN_Y(0),
                     COLOR_DARK_RED_HIGHLIGHT, 0, 0, buf);
}

extern int ric_update()
{
    enum ric_error err = RIC_SUCCESS;

    ///-------------------------------------------------------------------------
    //| Check for window resizes
    ///-------------------------------------------------------------------------
    ric_window_size(&SCREEN_WIDTH, &SCREEN_HEIGHT);

    ///-------------------------------------------------------------------------
    //| Query input state
    ///-------------------------------------------------------------------------
	// Get mouse state
    SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_buttons_prev = mouse_buttons;
	mouse_buttons = SDL_GetRelativeMouseState(&mouse_dx, &mouse_dy);

	//if (mouse_lock)
	//{
	//	mouse_dx = (r32)dx;
	//	mouse_dy = (r32)dy;
	//}

	// Get keyboard state
	u8 *keys_tmp = keys_prev;
	keys_prev = keys;
	keys = keys_tmp;
	memcpy(keys, SDL_GetKeyboardState(0), SDL_NUM_SCANCODES);

    ///-------------------------------------------------------------------------
    //| State actions
    ///-------------------------------------------------------------------------
    if (SDL_QuitRequested())
    {
        state = RIC_ENGINE_SHUTDOWN;
    }

    if (state != RIC_ENGINE_TEXT_INPUT)
    {
        rico_check_key_events();
    }

    state_last_frame = state;
    if (state_handlers[state].run)
    {
        err = state_handlers[state].run();
        if (err) return err;
    }

    RICO_ASSERT(state >= 0);
    if (state != state_last_frame)
    {
        if (state == RIC_ENGINE_SHUTDOWN)
            return RIC_SUCCESS;

        // Store last state before it changed
        state_prev = state_last_frame;

        // Clean up old state
        if (state_handlers[state_last_frame].cleanup)
        {
            err = state_handlers[state_last_frame].cleanup();
            if (err) return err;
        }
        // Initialize new state
        if (state_handlers[state].init)
        {
            err = state_handlers[state].init();
            if (err) return err;
        }

        // Render state label
        char buf[64] = { 0 };
        int len = snprintf(buf, sizeof(buf), "State: %d %s", state,
                           ric_engine_state_string[state]);
        string_truncate(buf, sizeof(buf), len);

        string_free_slot(RIC_STRING_SLOT_STATE);
        ric_load_string(RIC_PACK_ID_TRANSIENT, RIC_STRING_SLOT_STATE, SCREEN_X(0),
                         SCREEN_Y(0), COLOR_DARK_RED_HIGHLIGHT, 0, 0, buf);
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
        // TODO: Move this to this program.c
        // Update uniforms
        //RICO_ASSERT(global_prog_pbr->program.gl_id);
        //glUseProgram(global_prog_pbr->program.gl_id);
        //glUniform1f(global_prog_pbr->time, (r32)SIM_SEC);  // Cleanup: Shader time
        //glUseProgram(0);

        string_update();
        sim_accum -= SIM_MS;
    }
    sim_alpha = (r64)sim_accum / SIM_MS;

    // TODO: Handle mouse movements in the state callbacks. Camera movement
    //       is not the correct response to mouse movement in all states.
    if (mouse_lock && !ric_state_is_menu())
    {
        struct vec3 camera_acc = player_acc;
        if (player_sprint) v3_scalef(&camera_acc, CAM_SPRINT_MULTIPLIER);
        if (camera_slow)   v3_scalef(&camera_acc, CAM_SLOW_MULTIPLIER);
        camera_player_update(&cam_player, mouse_dx, mouse_dy, camera_acc,
                             (r32)frame_dt);
    }

    camera_update(&cam_player, sim_alpha);

    return err;
}
extern void ric_render_objects()
{
    struct program_pbr *prog = global_prog_pbr;

    // Render shadow maps
    render_shadow_cubemap(sim_alpha, prog->val.frag.lights);

    // Render scene
    object_render_all(sim_alpha, &cam_player);
}
extern void ric_render_editor()
{
    edit_render();
}
extern void ric_render_crosshair()
{
    camera_render(&cam_player);
}
extern void ric_frame_swap()
{
    platform_window_swap();

    // Clear previous frame
    //glClearColor(0.46f, 0.70f, 1.0f, 1.0f);
    //glClearDepth(0.0f);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if RICO_DEBUG
    // HACK: Kill some time (a.k.a. prevent my computer from lighting itself on
    //       fire when VSync is disabled)
    SDL_Delay(1);
#endif
}
extern void ric_render()
{
    ric_render_objects();
    ric_render_editor();
    // TODO: RICO_render_primitives();
    ric_render_crosshair();
}
static int shared_engine_events()
{
    enum ric_error err = RIC_SUCCESS;

#if RICO_DEBUG
    // DEBUG: Toggle scene lighting
    if (action_active(RIC_ACTION_ENGINE_DEBUG_LIGHTING_TOGGLE))
    {
        enable_lighting = !enable_lighting;
    }
    // DEBUG: Trigger breakpoint
    else if (action_active(RIC_ACTION_ENGINE_DEBUG_TRIGGER_BREAKPOINT))
    {
        SDL_TriggerBreakpoint();
    }
    // DEBUG: Change FOV
    else if (action_active(RIC_ACTION_ENGINE_DEBUG_FOV_INCREASE) ||
             action_active(RIC_ACTION_ENGINE_DEBUG_FOV_DECREASE))
    {
        float fov = cam_player.fov_deg;
        fov += action_active(RIC_ACTION_ENGINE_DEBUG_FOV_INCREASE) ? 1.0f : -1.0f;
        camera_set_fov(&cam_player, fov);
    }
#endif
    // Toggle FPS counter
    else if (action_active(RIC_ACTION_ENGINE_FPS_TOGGLE))
    {
        fps_render = !fps_render;
        if (!fps_render)
        {
            string_free_slot(RIC_STRING_SLOT_FPS);
        }
    }
    // Toggle mouse lock-to-window
    else if (action_active(RIC_ACTION_ENGINE_MOUSE_LOCK_TOGGLE))
    {
        mouse_lock = !mouse_lock;
        SDL_SetRelativeMouseMode(mouse_lock);
    }
    // Toggle vsync
    else if (action_active(RIC_ACTION_ENGINE_VSYNC_TOGGLE))
    {
        vsync = !vsync;
        // TODO: This causes stuttering, not sure how to make sure frame update
        //       loop stays in sync with monitor refresh.
        SDL_GL_SetSwapInterval(vsync);
    }
    // Toggle audio
    else if (action_active(RIC_ACTION_ENGINE_MUTE_TOGGLE))
    {
        ric_audio_toggle();

        char buf[16] = { 0 };
        int len;
        if (ric_audio_muted())
        {
            len = snprintf(buf, sizeof(buf), "Volume:   0%%");
        }
        else
        {
            len = snprintf(buf, sizeof(buf), "Volume: %3.f%%",
                           global_audio_volume * 100.0f);
        }
        string_truncate(buf, sizeof(buf), len);
        ric_load_string(RIC_PACK_ID_TRANSIENT, RIC_STRING_SLOT_COUNT,
                        SCREEN_X(-FONT_WIDTH * 12), SCREEN_Y(0), COLOR_GRAY_5,
                        1000, 0, buf);
    }
    // Pause simulation
    else if (action_active(RIC_ACTION_ENGINE_SIM_PAUSE))
    {
        sim_paused = !sim_paused;
    }
    // Save and exit
    else if (action_active(RIC_ACTION_ENGINE_QUIT))
    {
        ric_simulation_pause();
        string_free_slot(RIC_STRING_SLOT_MENU_QUIT);
        ric_load_string(RIC_PACK_ID_TRANSIENT, RIC_STRING_SLOT_MENU_QUIT,
                         SCREEN_X(SCREEN_WIDTH / 2 - 92),
                         SCREEN_Y(SCREEN_HEIGHT / 2 - 128),
                         COLOR_DARK_GREEN_HIGHLIGHT, 0, 0,
                         "                       \n" \
                         "  Save and quit?       \n" \
                         "                       \n" \
                         "  [Y] Yes              \n" \
                         "  [N] No               \n" \
                         "  [Q] Quit w/o saving  \n" \
                         "                       ");
        state = RIC_ENGINE_MENU_QUIT;
    }

    return err;
}
static int shared_camera_events()
{
    enum ric_error err = RIC_SUCCESS;

    player_acc = VEC3_ZERO;
    if (action_active(RIC_ACTION_MOVE_UP))       player_acc.y += 1.0f;
    if (action_active(RIC_ACTION_MOVE_DOWN))     player_acc.y -= 1.0f;
    if (action_active(RIC_ACTION_MOVE_RIGHT))    player_acc.x += 1.0f;
    if (action_active(RIC_ACTION_MOVE_LEFT))     player_acc.x -= 1.0f;
    if (action_active(RIC_ACTION_MOVE_FORWARD))  player_acc.z += 1.0f;
    if (action_active(RIC_ACTION_MOVE_BACKWARD)) player_acc.z -= 1.0f;
    v3_normalize(&player_acc);  // NOTE: Prevent faster movement on diagonal

    player_sprint = action_active(RIC_ACTION_MOVE_SPRINT);

    if (action_active(RIC_ACTION_CAMERA_SLOW_TOGGLE))
    {
        camera_slow = !camera_slow;
    }
    else if (action_active(RIC_ACTION_CAMERA_RESET))
    {
        camera_reset(&cam_player);
    }
    else if (action_active(RIC_ACTION_CAMERA_LOCK_TOGGLE))
    {
        cam_player.locked = !cam_player.locked;
    }
    else if (action_active(RIC_ACTION_CAMERA_WIREFRAME_TOGGLE))
    {
        cam_player.fill_mode = (cam_player.fill_mode == GL_FILL)
            ? GL_LINE
            : GL_FILL;
    }

    return err;
}

static int state_play_explore()
{
    enum ric_error err = RIC_SUCCESS;

    // Check global engine / editor chords after the more specific checks run
    // to allow overriding the global keybinds while in more specific states.
    // This is like doing base.Foo() at the end of an overridden Foo() method.
    // TODO: It might also be useful to have a stack of states, e.g.:
    // active_states = { ENGINE, EDITOR, TEXTURE }
    // active_states = { ENGINE, GAME, FLY }
    // Alternatively, stack can be used for "pushdown automata", where you
    // need to pop to a previous state when the current one finishes. e.g.:
    // state_stack.push(WALK);
    // state_stack.push(JUMP);
    // state_stack.pop(JUMP);
    err = shared_engine_events();
    if (err || state != state_last_frame) return err;

    err = shared_camera_events();
    if (err || state != state_last_frame) return err;

    // CLEANUP: Interact with clicked object
    // if (action_active(RIC_ACTION_PLAY_INTERACT))
    if (action_active(RIC_ACTION_PLAY_EDITOR))
    {
        state = RIC_ENGINE_EDIT_TRANSLATE;
        edit_print_object();
    }

    return err;
}

static int shared_edit_events()
{
    RICO_ASSERT(ric_state_is_edit());

    enum ric_error err = RIC_SUCCESS;

    bool cursor_moved = false;

    // Raycast object selection
    if (action_active(RIC_ACTION_EDIT_MOUSE_PICK_START))
    {
        edit_mouse_pressed();
    }
    else if (action_active(RIC_ACTION_EDIT_MOUSE_PICK_MOVE))
    {
        if (mouse_dx || mouse_dy)
        {
            cursor_moved = true;
        }
    }
    else if (action_active(RIC_ACTION_EDIT_MOUSE_PICK_END))
    {
        edit_mouse_released();
    }
    // Recalculate bounding boxes of all objects
    else if (action_active(RIC_ACTION_EDIT_BBOX_RECALCULATE_ALL))
    {
        edit_aabb_reset_all();
    }
    // Recalculate bounding box based on current mesh
    else if (action_active(RIC_ACTION_EDIT_BBOX_RECALCULATE))
    {
        edit_aabb_reset();
    }
    // Duplicate selected object
    else if (action_active(RIC_ACTION_EDIT_SELECTED_DUPLICATE))
    {
        edit_duplicate();
    }
    // Exit edit mode
    else if (action_active(RIC_ACTION_EDIT_QUIT))
    {
        edit_object_select(0, false);
        state = RIC_ENGINE_PLAY_EXPLORE;
    }
    // Select previous edit mode
    else if (action_active(RIC_ACTION_EDIT_MODE_PREVIOUS))
    {
        switch (state)
        {
            case RIC_ENGINE_EDIT_ROTATE:
            case RIC_ENGINE_EDIT_SCALE:
            case RIC_ENGINE_EDIT_MATERIAL:
            case RIC_ENGINE_EDIT_MESH:
                state--;
                break;
            case RIC_ENGINE_EDIT_TRANSLATE:
                state = RIC_ENGINE_EDIT_MESH;
                break;
            default:
                RICO_ASSERT("WTF");
        }
    }
    // Select next edit mode
    else if (action_active(RIC_ACTION_EDIT_MODE_NEXT))
    {
        switch (state)
        {
            case RIC_ENGINE_EDIT_TRANSLATE:
            case RIC_ENGINE_EDIT_ROTATE:
            case RIC_ENGINE_EDIT_SCALE:
            case RIC_ENGINE_EDIT_MATERIAL:
                state++;
                break;
            case RIC_ENGINE_EDIT_MESH:
                state = RIC_ENGINE_EDIT_TRANSLATE;
                break;
            default:
                RICO_ASSERT("WTF");
        }
    }
    // Cycle select through packs
    else if (action_active(RIC_ACTION_EDIT_CYCLE_PACK))
    {
        edit_pack_next();
    }
    // Cycle select through objects (in reverse)
    else if (action_active(RIC_ACTION_EDIT_CYCLE_BLOB_REVERSE))
    {
        edit_object_prev();
    }
    // Cycle select through objects
    else if (action_active(RIC_ACTION_EDIT_CYCLE_BLOB))
    {
        edit_object_next();
    }
    // Create new object
    else if (action_active(RIC_ACTION_EDIT_CREATE_OBJECT))
    {
        edit_object_create();
    }
    // Delete selected object
    else if (action_active(RIC_ACTION_EDIT_SELECTED_DELETE))
    {
        edit_delete();
    }
    // Save chunk
    else if (action_active(RIC_ACTION_EDIT_SAVE))
    {
        // TODO: Save all packs? Pack dirty flag?
        u32 pack_id = PKID_PACK(selected_obj_id);
        err = ric_pack_save(pack_id, false);
    }
    // Toggle camera project (perspective/orthographic)
    // TODO: Reset to perspective when leaving edit mode?
    else if (action_active(RIC_ACTION_EDIT_CAMERA_TOGGLE_PROJECTION))
    {
        camera_toggle_projection(&cam_player);
    }
    else if (action_active(RIC_ACTION_EDIT_DEBUG_TEXT_INPUT))
    {
        state = RIC_ENGINE_TEXT_INPUT;
    }

    if (cursor_moved || !v3_equals(&player_acc, &VEC3_ZERO))
    {
        edit_mouse_move();
    }

    return err;
}
static int state_edit_translate()
{
    enum ric_error err = RIC_SUCCESS;

    err = shared_edit_events();
    if (err || state != state_last_frame) return err;
    err = shared_engine_events();
    if (err || state != state_last_frame) return err;
    err = shared_camera_events();
    if (err || state != state_last_frame) return err;

    struct vec3 translate = VEC3_ZERO;
    bool translate_reset = false;
    bool trans_delta_changed = false;

    // Reset selected object's translation
    if (action_active(RIC_ACTION_EDIT_TRANSLATE_RESET))
        translate_reset = true;
    // Translate selected object up
    else if (action_active(RIC_ACTION_EDIT_TRANSLATE_UP))
        translate.y += global_trans_delta;
    // Translate selected object down
    else if (action_active(RIC_ACTION_EDIT_TRANSLATE_DOWN))
        translate.y -= global_trans_delta;
    // Translate selected object west
    else if (action_active(RIC_ACTION_EDIT_TRANSLATE_WEST))
        translate.x -= global_trans_delta;
    // Translate selected object east
    else if (action_active(RIC_ACTION_EDIT_TRANSLATE_EAST))
        translate.x += global_trans_delta;
    // Translate selected object north
    else if (action_active(RIC_ACTION_EDIT_TRANSLATE_NORTH))
        translate.z -= global_trans_delta;
    // Translate selected object south
    else if (action_active(RIC_ACTION_EDIT_TRANSLATE_SOUTH))
        translate.z += global_trans_delta;
    // Increase translation delta
    else if (action_active(RIC_ACTION_EDIT_TRANSLATE_DELTA_INCREASE))
    {
        if (global_trans_delta < TRANS_DELTA_MAX)
        {
            global_trans_delta *= 10.0f;
            if (global_trans_delta > TRANS_DELTA_MAX)
                global_trans_delta = TRANS_DELTA_MAX;

            trans_delta_changed = true;
        }
    }
    // Decrease translation delta
    else if (action_active(RIC_ACTION_EDIT_TRANSLATE_DELTA_DECREASE))
    {
        if (global_trans_delta > TRANS_DELTA_MIN)
        {
            global_trans_delta /= 10.0f;
            if (global_trans_delta < TRANS_DELTA_MIN)
                global_trans_delta = TRANS_DELTA_MIN;

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
        int len = snprintf(buf, sizeof(buf), "Trans Delta: %f", global_trans_delta);
        string_truncate(buf, sizeof(buf), len);
        string_free_slot(RIC_STRING_SLOT_DELTA);
        ric_load_string(RIC_PACK_ID_TRANSIENT, RIC_STRING_SLOT_DELTA,
                         SCREEN_X(0), SCREEN_Y(0), COLOR_DARK_BLUE_HIGHLIGHT,
                         1000, 0, buf);
    }

    return err;
}
static int state_edit_rotate()
{
    enum ric_error err = RIC_SUCCESS;

    err = shared_edit_events();
    if (err || state != state_last_frame) return err;
    err = shared_engine_events();
    if (err || state != state_last_frame) return err;
    err = shared_camera_events();
    if (err || state != state_last_frame) return err;

    struct vec3 rotate = VEC3_ZERO;
    bool rotate_reset = false;
    bool rot_delta_changed = false;

    // Toggle heptagonal rotations
    if (action_active(RIC_ACTION_EDIT_ROTATE_HEPTAMODE_TOGGLE))
    {
        if (global_rot_delta == ROT_DELTA_DEFAULT)
            global_rot_delta = (float)M_SEVENTH_DEG;
        else
            global_rot_delta = ROT_DELTA_DEFAULT;
    }
    // Reset selected object's rotation
    else if (action_active(RIC_ACTION_EDIT_ROTATE_RESET))
        rotate_reset = true;
    // Rotate selected object CCW around positive x-axis
    else if (action_active(RIC_ACTION_EDIT_ROTATE_X_POS))
        rotate.x += global_rot_delta;
    // Rotate selected object CCW around negative x-axis
    else if (action_active(RIC_ACTION_EDIT_ROTATE_X_NEG))
        rotate.x -= global_rot_delta;
    // Rotate selected object CCW around positive y-axis
    else if (action_active(RIC_ACTION_EDIT_ROTATE_Y_NEG))
        rotate.y += global_rot_delta;
    // Rotate selected object CCW around negative y-axis
    else if (action_active(RIC_ACTION_EDIT_ROTATE_Y_POS))
        rotate.y -= global_rot_delta;
    // Rotate selected object CCW around positive z-axis
    else if (action_active(RIC_ACTION_EDIT_ROTATE_Z_POS))
        rotate.z += global_rot_delta;
    // Rotate selected object CCW around negative z-axis
    else if (action_active(RIC_ACTION_EDIT_ROTATE_Z_NEG))
        rotate.z -= global_rot_delta;
    // Increase rotation delta
    else if (action_active(RIC_ACTION_EDIT_ROTATE_DELTA_INCREASE))
    {
        if (global_rot_delta < ROT_DELTA_MAX)
        {
            global_rot_delta += (global_rot_delta < 5.0f) ? 1.0f : 5.0f;
            if (global_rot_delta > ROT_DELTA_MAX)
                global_rot_delta = ROT_DELTA_MAX;

            rot_delta_changed = true;
        }
    }
    // Decrease rotation delta
    else if (action_active(RIC_ACTION_EDIT_ROTATE_DELTA_DECREASE))
    {
        if (global_rot_delta > ROT_DELTA_MIN)
        {
            global_rot_delta -= (global_rot_delta > 5.0f) ? 5.0f : 1.0f;
            if (global_rot_delta < ROT_DELTA_MIN)
                global_rot_delta = ROT_DELTA_MIN;

            rot_delta_changed = true;
        }
    }

    // Update selected object
    if (rotate_reset || !v3_equals(&rotate, &VEC3_ZERO))
    {
        struct quat qx, qy, qz;
        quat_from_axis_angle(&qx, VEC3_X, rotate.x);
        quat_from_axis_angle(&qy, VEC3_Y, rotate.y);
        quat_from_axis_angle(&qz, VEC3_Z, rotate.z);
        quat_mul(&qx, &qy);
        quat_mul(&qx, &qz);
        edit_rotate(&qx);
    }

    if (rot_delta_changed)
    {
        char buf[32] = { 0 };
        int len = snprintf(buf, sizeof(buf), "Rot Delta: %f", global_rot_delta);
        string_truncate(buf, sizeof(buf), len);

        string_free_slot(RIC_STRING_SLOT_DELTA);
        ric_load_string(RIC_PACK_ID_TRANSIENT, RIC_STRING_SLOT_DELTA,
                         SCREEN_X(0), SCREEN_Y(0), COLOR_DARK_BLUE_HIGHLIGHT,
                         1000, 0, buf);
    }

    return err;
}
static int state_edit_scale()
{
    enum ric_error err = RIC_SUCCESS;

    err = shared_edit_events();
    if (err || state != state_last_frame) return err;
    err = shared_engine_events();
    if (err || state != state_last_frame) return err;
    err = shared_camera_events();
    if (err || state != state_last_frame) return err;

    struct vec3 scale = VEC3_ZERO;
    bool scale_reset = false;
    bool scale_delta_changed = false;

    // Reset selected object's scale
    if (action_active(RIC_ACTION_EDIT_SCALE_RESET))
    {
        scale_reset = true;
    }
    // Scale selected object up
    else if (action_active(RIC_ACTION_EDIT_SCALE_UP))
    {
        scale.y += global_scale_delta;
    }
    // Scale selected object down
    else if (action_active(RIC_ACTION_EDIT_SCALE_DOWN))
    {
        scale.y -= global_scale_delta;
    }
    // Scale selected object west
    else if (action_active(RIC_ACTION_EDIT_SCALE_WEST))
    {
        scale.x -= global_scale_delta;
    }
    // Scale selected object east
    else if (action_active(RIC_ACTION_EDIT_SCALE_EAST))
    {
        scale.x += global_scale_delta;
    }
    // Scale selected object north
    else if (action_active(RIC_ACTION_EDIT_SCALE_NORTH))
    {
        scale.z += global_scale_delta;
    }
    // Scale selected object south
    else if (action_active(RIC_ACTION_EDIT_SCALE_SOUTH))
    {
        scale.z -= global_scale_delta;
    }
    // Increase scale delta
    else if (action_active(RIC_ACTION_EDIT_SCALE_DELTA_INCREASE))
    {
        if (global_scale_delta < SCALE_DELTA_MAX)
        {
            global_scale_delta += (global_scale_delta < 1.0f) ? 0.1f : 1.0f;
            if (global_scale_delta > SCALE_DELTA_MAX)
                global_scale_delta = SCALE_DELTA_MAX;

            scale_delta_changed = true;
        }
    }
    // Decrease scale delta
    else if (action_active(RIC_ACTION_EDIT_SCALE_DELTA_DECREASE))
    {
        if (global_scale_delta > SCALE_DELTA_MIN)
        {
            global_scale_delta -= (global_scale_delta > 1.0f) ? 1.0f : 0.1f;
            if (global_scale_delta < SCALE_DELTA_MIN)
                global_scale_delta = SCALE_DELTA_MIN;

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
        int len = snprintf(buf, sizeof(buf), "Scale Delta: %f", global_scale_delta);
        string_truncate(buf, sizeof(buf), len);
        string_free_slot(RIC_STRING_SLOT_DELTA);
        ric_load_string(RIC_PACK_ID_TRANSIENT, RIC_STRING_SLOT_DELTA,
                         SCREEN_X(0), SCREEN_Y(0), COLOR_DARK_BLUE_HIGHLIGHT,
                         1000, 0, buf);
    }

    return err;
}
static int state_edit_material()
{
    enum ric_error err = RIC_SUCCESS;

    err = shared_edit_events();
    if (err || state != state_last_frame) return err;
    err = shared_engine_events();
    if (err || state != state_last_frame) return err;
    err = shared_camera_events();
    if (err || state != state_last_frame) return err;

    // Cycle selected object's material
    if (action_active(RIC_ACTION_EDIT_MATERIAL_NEXT))
    {
        edit_material_next();
    }
    // Cycle selected object's material (in reverse)
    else if (action_active(RIC_ACTION_EDIT_MATERIAL_PREVIOUS))
    {
        edit_material_prev();
    }
    // Cycle all packs looking for materials
    else if (action_active(RIC_ACTION_EDIT_MATERIAL_NEXT_PACK))
    {
        edit_material_next_pack();
    }

    return err;
}
static int state_edit_mesh()
{
    enum ric_error err = RIC_SUCCESS;

    err = shared_edit_events();
    if (err || state != state_last_frame) return err;
    err = shared_engine_events();
    if (err || state != state_last_frame) return err;
    err = shared_camera_events();
    if (err || state != state_last_frame) return err;

    // Cycle selected object's mesh
    if (action_active(RIC_ACTION_EDIT_MESH_NEXT))
    {
        edit_mesh_next();
    }
    // Cycle selected object's mesh (in reverse)
    else if (action_active(RIC_ACTION_EDIT_MESH_PREVIOUS))
    {
        edit_mesh_prev();
    }
    // Cycle all packs looking for meshes
    else if (action_active(RIC_ACTION_EDIT_MESH_NEXT_PACK))
    {
        edit_mesh_next_pack();
    }

    return err;
}
static int state_menu_quit()
{
    enum ric_error err = RIC_SUCCESS;

    // [Y] / [Return]: Save and exit
    if (KEY_PRESSED(SDL_SCANCODE_Y) || KEY_PRESSED(SDL_SCANCODE_RETURN))
    {
        string_free_slot(RIC_STRING_SLOT_MENU_QUIT);
        // TODO: Save all packs? Pack dirty flag?
        u32 pack_id = PKID_PACK(selected_obj_id);
		err = ric_pack_save(pack_id, false);
        state = RIC_ENGINE_SHUTDOWN;
    }
    // [N] / [Escape]: Return to play mode
    else if (KEY_PRESSED(SDL_SCANCODE_N) || KEY_PRESSED(SDL_SCANCODE_ESCAPE))
    {
        ric_simulation_prev();
        string_free_slot(RIC_STRING_SLOT_MENU_QUIT);
        state = state_prev;
    }
    // [Q]: Exit without saving
    else if (KEY_PRESSED(SDL_SCANCODE_Q))
    {
        string_free_slot(RIC_STRING_SLOT_MENU_QUIT);
        state = RIC_ENGINE_SHUTDOWN;
    }

    return err;
}
static int state_edit_cleanup()
{
    enum ric_error err = RIC_SUCCESS;

    if (ric_state_is_edit())
    {
        string_free_slot(RIC_STRING_SLOT_SELECTED_OBJ);
    }

    return err;
}

static int state_text_input_init()
{
    enum ric_error err = RIC_SUCCESS;

    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    SDL_StartTextInput();
    SDL_SetRelativeMouseMode(false);

    return err;
}
static int state_text_input()
{
    enum ric_error err = RIC_SUCCESS;

    if (KEY_PRESSED(SDL_SCANCODE_ESCAPE))
    {
        RICO_ASSERT(state != state_prev);
        state = state_prev;
    }

    return err;
}
static int state_text_input_cleanup()
{
    enum ric_error err = RIC_SUCCESS;

    RICO_ASSERT(SDL_IsTextInputActive());
    SDL_StopTextInput();
    SDL_SetRelativeMouseMode(mouse_lock);

    return err;
}

static int rico_init_shaders()
{
    enum ric_error err;

    // Create shader programs
    err = make_program_pbr(&global_prog_pbr);
    if (err) return err;

    err = make_program_shadow_texture(&global_prog_shadow_texture);
    if (err) return err;

    err = make_program_shadow_cubemap(&global_prog_shadow_cubemap);
    if (err) return err;

    err = make_program_primitive(&global_prog_primitive);
    if (err) return err;

    err = make_program_text(&global_prog_text);
    if (err) return err;

    // TODO: Lights should be objects around the world and the shader should
    //       render the 4 closest ones to the player's position.
    const float INTENSITY = 0.000001f;

    global_prog_pbr->val.frag.lights[0].type = RIC_LIGHT_DIRECTIONAL;
    global_prog_pbr->val.frag.lights[0].directional.direction = VEC3(-1.0f, -1.0f, 0.0f);
    v3_normalize(&global_prog_pbr->val.frag.lights[0].directional.direction);

    global_prog_pbr->val.frag.lights[1].type = RIC_LIGHT_POINT;
    global_prog_pbr->val.frag.lights[1].position = VEC3(0.0f, 4.0f, 0.0f);

    global_prog_pbr->val.frag.lights[2].type = RIC_LIGHT_POINT;
    global_prog_pbr->val.frag.lights[3].type = RIC_LIGHT_POINT;
    global_prog_pbr->val.frag.lights[4].type = RIC_LIGHT_POINT;

    //global_prog_pbr->val.frag.lights[2].position = VEC3(-4.0f, 4.0f, 3.0f);
    //global_prog_pbr->val.frag.lights[3].position = VEC3(4.0f, 5.0f, 3.0f);
    //global_prog_pbr->val.frag.lights[4].position = VEC3(1.0f, 6.0f, 3.0f);
    global_prog_pbr->val.frag.lights[2].position = VEC3(-1.0f, 4.0f, 3.0f);
    global_prog_pbr->val.frag.lights[3].position = VEC3(0.0f, 4.0f, 3.0f);
    global_prog_pbr->val.frag.lights[4].position = VEC3(1.0f, 4.0f, 3.0f);

    global_prog_pbr->val.frag.lights[0].color = VEC3(1.0f, 1.0f, 1.0f);
    global_prog_pbr->val.frag.lights[1].color = VEC3(1.0f, 1.0f, 1.0f);
    global_prog_pbr->val.frag.lights[2].color = VEC3(1.0f, 0.0f, 0.0f);
    global_prog_pbr->val.frag.lights[3].color = VEC3(0.0f, 1.0f, 0.0f);
    global_prog_pbr->val.frag.lights[4].color = VEC3(0.0f, 0.0f, 1.0f);
    global_prog_pbr->val.frag.lights[0].intensity = 1.0f;
    global_prog_pbr->val.frag.lights[1].intensity = INTENSITY;
    global_prog_pbr->val.frag.lights[2].intensity = INTENSITY;
    global_prog_pbr->val.frag.lights[3].intensity = INTENSITY;
    global_prog_pbr->val.frag.lights[4].intensity = INTENSITY;
    global_prog_pbr->val.frag.lights[0].on = global_lighting_enabled && true;
    global_prog_pbr->val.frag.lights[1].on = global_lighting_enabled && false;
    global_prog_pbr->val.frag.lights[2].on = global_lighting_enabled && false;
    global_prog_pbr->val.frag.lights[3].on = global_lighting_enabled && false;
    global_prog_pbr->val.frag.lights[4].on = global_lighting_enabled && false;
    //global_prog_pbr->val.frag.light.kc = 1.0f;
    //global_prog_pbr->val.frag.light.kl = 0.05f;
    //global_prog_pbr->val.frag.light.kq = 0.001f;

    return err;
}
static int state_engine_shutdown()
{
	edit_free();
	return RIC_SUCCESS;
}
static void rico_check_key_events()
{
    // NOTE: Starting at 1 because action 0 is RIC_ACTION_NULL
    u32 i = 1;
    while (i < action_max && action_queue_count < ARRAY_COUNT(action_queue))
    {
        // Trigger action if chord is down and repeat enabled, or chord down
        // and wasn't down last frame (i.e. chord pressed).
        if (action_active(i))
        {
            action_queue[action_queue_count] = i;
            action_queue_count++;
        }
        i++;
    }
}
extern bool ric_quit()
{
    return state == RIC_ENGINE_SHUTDOWN;
}
extern void ric_mouse_coords(u32 *x, u32 *y)
{
    *x = mouse_x;
    *y = mouse_y;
}
extern u32 ric_key_event(u32 *action)
{
    bool has_action = false;
    if (action_queue_count)
    {
        // TODO: Circular queue
        action_queue_count--;
        *action = action_queue[action_queue_count];
        action_queue[action_queue_count] = 0;
        has_action = true;
    }
    return has_action;
}
extern void ric_bind_action(u32 action, struct ric_keychord chord)
{
    RICO_ASSERT(action < ARRAY_COUNT(action_chords));
    action_chords[action] = chord;
    action_max = MAX(action_max, action);
}
static int engine_init()
{
    enum ric_error err;

    printf("=========================================================\n");
    printf("#        ______            _______        _             #\n");
    printf("#        |  __ \\ O        |__   __|      | |            #\n");
    printf("#        | |__| |_  ___ ___  | | ___  ___| |__          #\n");
    printf("#        |  _  /| |/ __/ _ \\ | |/ _ \\/ __| '_ \\         #\n");
    printf("#        | | \\ \\| | |_| (_) || |  __/ |__| | | |        #\n");
    printf("#        |_|  \\_\\_|\\___\\___/ |_|\\___|\\___|_| |_|        #\n");
    printf("#                                                       #\n");
    printf("#              Copyright 2018  Dan Bechard              #\n");
    printf("=========================================================\n");

    perfs_frequency = SDL_GetPerformanceFrequency();
    init_perfs = SDL_GetPerformanceCounter();
    last_perfs = init_perfs;
    last_cycles = __rdtsc();
    fps_last_render = last_perfs;

    int sdl_err = SDL_GL_SetSwapInterval(vsync);
    if (sdl_err < 0) {
        return RICO_ERROR(RIC_ERR_SDL_INIT, "SDL_GL_SetSwapInterval error: %s",
                          SDL_GetError());
    }

    // Initialize mouse and keyboard states
	SDL_GetKeyboardState(0);
	SDL_GetRelativeMouseState(0, 0);
    SDL_SetRelativeMouseMode(mouse_lock);
    mouse_dx = 0;
    mouse_dy = 0;

    // TODO: Load from config file?
    // Initialize key map

    // Engine
    ric_bind_action(RIC_ACTION_ENGINE_DEBUG_LIGHTING_TOGGLE,    RIC_CHORD1(SDL_SCANCODE_L));
    ric_bind_action(RIC_ACTION_ENGINE_DEBUG_TRIGGER_BREAKPOINT, RIC_CHORD1(SDL_SCANCODE_F9));
    ric_bind_action(RIC_ACTION_ENGINE_DEBUG_FOV_INCREASE,       RIC_CHORD1(SDL_SCANCODE_3));
    ric_bind_action(RIC_ACTION_ENGINE_DEBUG_FOV_DECREASE,       RIC_CHORD1(SDL_SCANCODE_4));
    ric_bind_action(RIC_ACTION_ENGINE_FPS_TOGGLE,               RIC_CHORD1(SDL_SCANCODE_2));
    ric_bind_action(RIC_ACTION_ENGINE_MOUSE_LOCK_TOGGLE,        RIC_CHORD1(SDL_SCANCODE_M));
    ric_bind_action(RIC_ACTION_ENGINE_VSYNC_TOGGLE,             RIC_CHORD1(SDL_SCANCODE_V));
    ric_bind_action(RIC_ACTION_ENGINE_MUTE_TOGGLE,              RIC_CHORD1(SDL_SCANCODE_PERIOD));
    ric_bind_action(RIC_ACTION_ENGINE_SIM_PAUSE,                RIC_CHORD1(SDL_SCANCODE_P));
    ric_bind_action(RIC_ACTION_ENGINE_QUIT,                     RIC_CHORD1(SDL_SCANCODE_ESCAPE));

    // Camera
    ric_bind_action(RIC_ACTION_CAMERA_SLOW_TOGGLE,              RIC_CHORD1(SDL_SCANCODE_R));
    ric_bind_action(RIC_ACTION_CAMERA_RESET,                    RIC_CHORD1(SDL_SCANCODE_F));
    ric_bind_action(RIC_ACTION_CAMERA_LOCK_TOGGLE,              RIC_CHORD1(SDL_SCANCODE_C));
    ric_bind_action(RIC_ACTION_CAMERA_WIREFRAME_TOGGLE,         RIC_CHORD1(SDL_SCANCODE_1));

    // Player
    ric_bind_action(RIC_ACTION_MOVE_RIGHT,                      RIC_CHORD_REPEAT1(SDL_SCANCODE_D));
    ric_bind_action(RIC_ACTION_MOVE_LEFT,                       RIC_CHORD_REPEAT1(SDL_SCANCODE_A));
    ric_bind_action(RIC_ACTION_MOVE_UP,                         RIC_CHORD_REPEAT1(SDL_SCANCODE_E));
    ric_bind_action(RIC_ACTION_MOVE_DOWN,                       RIC_CHORD_REPEAT1(SDL_SCANCODE_Q));
    ric_bind_action(RIC_ACTION_MOVE_FORWARD,                    RIC_CHORD_REPEAT1(SDL_SCANCODE_W));
    ric_bind_action(RIC_ACTION_MOVE_BACKWARD,                   RIC_CHORD_REPEAT1(SDL_SCANCODE_S));
    ric_bind_action(RIC_ACTION_MOVE_SPRINT,                     RIC_CHORD_REPEAT1(RIC_SCANCODE_SHIFT));

    ric_bind_action(RIC_ACTION_PLAY_EDITOR,                     RIC_CHORD1(SDL_SCANCODE_GRAVE));
    ric_bind_action(RIC_ACTION_PLAY_INTERACT,                   RIC_CHORD1(RIC_SCANCODE_LMB));

    // Editor
    ric_bind_action(RIC_ACTION_EDIT_QUIT,                       RIC_CHORD1(SDL_SCANCODE_GRAVE));
    ric_bind_action(RIC_ACTION_EDIT_SAVE,                       RIC_CHORD2(RIC_SCANCODE_CTRL,   SDL_SCANCODE_S));
    ric_bind_action(RIC_ACTION_EDIT_CYCLE_PACK,                 RIC_CHORD2(RIC_SCANCODE_CTRL,   SDL_SCANCODE_TAB));
    ric_bind_action(RIC_ACTION_EDIT_CYCLE_BLOB_REVERSE,         RIC_CHORD2(RIC_SCANCODE_SHIFT,  SDL_SCANCODE_TAB));
    ric_bind_action(RIC_ACTION_EDIT_CYCLE_BLOB,                 RIC_CHORD1(SDL_SCANCODE_TAB));
    ric_bind_action(RIC_ACTION_EDIT_MOUSE_PICK_START,           RIC_CHORD1(RIC_SCANCODE_LMB));
    ric_bind_action(RIC_ACTION_EDIT_MOUSE_PICK_MOVE,            RIC_CHORD_REPEAT1(RIC_SCANCODE_LMB));
    ric_bind_action(RIC_ACTION_EDIT_MOUSE_PICK_END,             RIC_CHORD_UP1(RIC_SCANCODE_LMB));
    ric_bind_action(RIC_ACTION_EDIT_BBOX_RECALCULATE_ALL,       RIC_CHORD2(RIC_SCANCODE_SHIFT,  SDL_SCANCODE_B));
    ric_bind_action(RIC_ACTION_EDIT_BBOX_RECALCULATE,           RIC_CHORD1(SDL_SCANCODE_B));
    ric_bind_action(RIC_ACTION_EDIT_CREATE_OBJECT,              RIC_CHORD1(SDL_SCANCODE_INSERT));
    ric_bind_action(RIC_ACTION_EDIT_SELECTED_DUPLICATE,         RIC_CHORD2(RIC_SCANCODE_CTRL,   SDL_SCANCODE_D));
    ric_bind_action(RIC_ACTION_EDIT_SELECTED_DELETE,            RIC_CHORD1(SDL_SCANCODE_DELETE));
    ric_bind_action(RIC_ACTION_EDIT_MODE_PREVIOUS,              RIC_CHORD1(SDL_SCANCODE_KP_PERIOD));
    ric_bind_action(RIC_ACTION_EDIT_MODE_NEXT,                  RIC_CHORD1(SDL_SCANCODE_KP_0));
    ric_bind_action(RIC_ACTION_EDIT_CAMERA_TOGGLE_PROJECTION,   RIC_CHORD1(SDL_SCANCODE_O));
    ric_bind_action(RIC_ACTION_EDIT_DEBUG_TEXT_INPUT,           RIC_CHORD1(SDL_SCANCODE_I));

    ric_bind_action(RIC_ACTION_EDIT_TRANSLATE_RESET,            RIC_CHORD1(SDL_SCANCODE_0));
    ric_bind_action(RIC_ACTION_EDIT_TRANSLATE_UP,               RIC_CHORD_REPEAT1(SDL_SCANCODE_PAGEUP));
    ric_bind_action(RIC_ACTION_EDIT_TRANSLATE_DOWN,             RIC_CHORD_REPEAT1(SDL_SCANCODE_PAGEDOWN));
    ric_bind_action(RIC_ACTION_EDIT_TRANSLATE_WEST,             RIC_CHORD_REPEAT1(SDL_SCANCODE_LEFT));
    ric_bind_action(RIC_ACTION_EDIT_TRANSLATE_EAST,             RIC_CHORD_REPEAT1(SDL_SCANCODE_RIGHT));
    ric_bind_action(RIC_ACTION_EDIT_TRANSLATE_NORTH,            RIC_CHORD_REPEAT1(SDL_SCANCODE_UP));
    ric_bind_action(RIC_ACTION_EDIT_TRANSLATE_SOUTH,            RIC_CHORD_REPEAT1(SDL_SCANCODE_DOWN));
    ric_bind_action(RIC_ACTION_EDIT_TRANSLATE_DELTA_INCREASE,   RIC_CHORD1(SDL_SCANCODE_KP_PLUS));
    ric_bind_action(RIC_ACTION_EDIT_TRANSLATE_DELTA_DECREASE,   RIC_CHORD1(SDL_SCANCODE_KP_MINUS));

    ric_bind_action(RIC_ACTION_EDIT_ROTATE_RESET,               RIC_CHORD1(SDL_SCANCODE_0));
    ric_bind_action(RIC_ACTION_EDIT_ROTATE_X_POS,               RIC_CHORD_REPEAT1(SDL_SCANCODE_DOWN));
    ric_bind_action(RIC_ACTION_EDIT_ROTATE_X_NEG,               RIC_CHORD_REPEAT1(SDL_SCANCODE_UP));
    ric_bind_action(RIC_ACTION_EDIT_ROTATE_Y_POS,               RIC_CHORD_REPEAT1(SDL_SCANCODE_PAGEUP));
    ric_bind_action(RIC_ACTION_EDIT_ROTATE_Y_NEG,               RIC_CHORD_REPEAT1(SDL_SCANCODE_PAGEDOWN));
    ric_bind_action(RIC_ACTION_EDIT_ROTATE_Z_POS,               RIC_CHORD_REPEAT1(SDL_SCANCODE_LEFT));
    ric_bind_action(RIC_ACTION_EDIT_ROTATE_Z_NEG,               RIC_CHORD_REPEAT1(SDL_SCANCODE_RIGHT));
    ric_bind_action(RIC_ACTION_EDIT_ROTATE_DELTA_INCREASE,      RIC_CHORD1(SDL_SCANCODE_KP_PLUS));
    ric_bind_action(RIC_ACTION_EDIT_ROTATE_DELTA_DECREASE,      RIC_CHORD1(SDL_SCANCODE_KP_MINUS));
    ric_bind_action(RIC_ACTION_EDIT_ROTATE_HEPTAMODE_TOGGLE,    RIC_CHORD1(SDL_SCANCODE_7));

    ric_bind_action(RIC_ACTION_EDIT_SCALE_RESET,                RIC_CHORD1(SDL_SCANCODE_0));
    ric_bind_action(RIC_ACTION_EDIT_SCALE_UP,                   RIC_CHORD_REPEAT1(SDL_SCANCODE_UP));
    ric_bind_action(RIC_ACTION_EDIT_SCALE_DOWN,                 RIC_CHORD_REPEAT1(SDL_SCANCODE_DOWN));
    ric_bind_action(RIC_ACTION_EDIT_SCALE_WEST,                 RIC_CHORD_REPEAT1(SDL_SCANCODE_LEFT));
    ric_bind_action(RIC_ACTION_EDIT_SCALE_EAST,                 RIC_CHORD_REPEAT1(SDL_SCANCODE_RIGHT));
    ric_bind_action(RIC_ACTION_EDIT_SCALE_NORTH,                RIC_CHORD_REPEAT1(SDL_SCANCODE_PAGEUP));
    ric_bind_action(RIC_ACTION_EDIT_SCALE_SOUTH,                RIC_CHORD_REPEAT1(SDL_SCANCODE_PAGEDOWN));
    ric_bind_action(RIC_ACTION_EDIT_SCALE_DELTA_INCREASE,       RIC_CHORD1(SDL_SCANCODE_KP_PLUS));
    ric_bind_action(RIC_ACTION_EDIT_SCALE_DELTA_DECREASE,       RIC_CHORD1(SDL_SCANCODE_KP_MINUS));

    ric_bind_action(RIC_ACTION_EDIT_MATERIAL_NEXT,              RIC_CHORD1(SDL_SCANCODE_RIGHT));
    ric_bind_action(RIC_ACTION_EDIT_MATERIAL_PREVIOUS,          RIC_CHORD1(SDL_SCANCODE_LEFT));
    ric_bind_action(RIC_ACTION_EDIT_MATERIAL_NEXT_PACK,         RIC_CHORD1(SDL_SCANCODE_UP));

    ric_bind_action(RIC_ACTION_EDIT_MESH_NEXT,                  RIC_CHORD1(SDL_SCANCODE_RIGHT));
    ric_bind_action(RIC_ACTION_EDIT_MESH_PREVIOUS,              RIC_CHORD1(SDL_SCANCODE_LEFT));
    ric_bind_action(RIC_ACTION_EDIT_MESH_NEXT_PACK,             RIC_CHORD1(SDL_SCANCODE_UP));

    state_handlers[RIC_ENGINE_TEXT_INPUT     ].init = &state_text_input_init;

	state_handlers[RIC_ENGINE_SHUTDOWN].run = &state_engine_shutdown;
	state_handlers[RIC_ENGINE_MENU_QUIT      ].run = &state_menu_quit;
	state_handlers[RIC_ENGINE_PLAY_EXPLORE   ].run = &state_play_explore;
	state_handlers[RIC_ENGINE_EDIT_TRANSLATE ].run = &state_edit_translate;
	state_handlers[RIC_ENGINE_EDIT_ROTATE    ].run = &state_edit_rotate;
	state_handlers[RIC_ENGINE_EDIT_SCALE     ].run = &state_edit_scale;
	state_handlers[RIC_ENGINE_EDIT_MATERIAL  ].run = &state_edit_material;
	state_handlers[RIC_ENGINE_EDIT_MESH      ].run = &state_edit_mesh;
    state_handlers[RIC_ENGINE_TEXT_INPUT     ].run = &state_text_input;

	state_handlers[RIC_ENGINE_EDIT_TRANSLATE ].cleanup = &state_edit_cleanup;
	state_handlers[RIC_ENGINE_EDIT_ROTATE    ].cleanup = &state_edit_cleanup;
	state_handlers[RIC_ENGINE_EDIT_SCALE     ].cleanup = &state_edit_cleanup;
	state_handlers[RIC_ENGINE_EDIT_MATERIAL  ].cleanup = &state_edit_cleanup;
    state_handlers[RIC_ENGINE_EDIT_MESH      ].cleanup = &state_edit_cleanup;
    state_handlers[RIC_ENGINE_TEXT_INPUT     ].cleanup = &state_text_input_cleanup;

	state = RIC_ENGINE_PLAY_EXPLORE;

	printf("----------------------------------------------------------\n");
	printf("[MAIN][init] Initializing storage\n");
	printf("----------------------------------------------------------\n");
    rico_resource_init();
    rico_texture_init();
    rico_mesh_init();
    rico_ui_init();

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing heiro\n");
    printf("----------------------------------------------------------\n");
    err = rico_heiro_init();
    if (err) return err;

	printf("----------------------------------------------------------\n");
	printf("[MAIN][init] Initializing shaders\n");
	printf("----------------------------------------------------------\n");
	err = rico_init_shaders();
	if (err) return err;

	printf("----------------------------------------------------------\n");
	printf("[MAIN][init] Initializing editor\n");
	printf("----------------------------------------------------------\n");
	edit_init();

	printf("----------------------------------------------------------\n");
	printf("[MAIN][init] Initializing packs\n");
	printf("----------------------------------------------------------\n");
	//err = ric_pack_load("packs/default.pak", 0);
	//if (err) return err;
	ric_pack_init(RIC_PACK_ID_TRANSIENT, "pack_transient", 512, MB(4));
	ric_pack_init(RIC_PACK_ID_FRAME, "pack_frame", 0, 0);

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing primitives\n");
    printf("----------------------------------------------------------\n");
    prim_init();

	printf("----------------------------------------------------------\n");
	printf("[MAIN][init] Initializing camera\n");
	printf("----------------------------------------------------------\n");
	camera_reset(&cam_player);
	if (err) return err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][  GO] Initialization complete. Starting game.\n");
    printf("----------------------------------------------------------\n");

    state = RIC_ENGINE_PLAY_EXPLORE;
    return err;
}
