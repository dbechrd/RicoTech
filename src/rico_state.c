
const char *rico_state_string[] = {
    RICO_STATES(GEN_STRING)
};

enum rico_key {
    RICOKEY_RIGHT,
    RICOKEY_LEFT,
    RICOKEY_UP,
    RICOKEY_DOWN,
    RICOKEY_FORWARD,
    RICOKEY_BACKWARD,
    RICOKEY_SPRINT,
    RICOKEY_CAMERA_SLOW,
    RICOKEY_CAMERA_RESET,
    RICOKEY_CAMERA_LOCK,
    RICOKEY_CAMERA_WIREFRAME,
    RICOKEY_COUNT
};

////////////////////////////////////////////////////////////////////////////////
global const bool reset_game_world = true;
global struct rico_chunk *RICO_DEFAULT_CHUNK;

////////////////////////////////////////////////////////////////////////////////
//Human walk speed empirically found to be 33 steps in 20 seconds. That
//is approximately 1.65 steps per second. At 60 fps, that is 0.0275 steps
//per frame. Typical walking stride is ~0.762 meters (30 inches). Distance
//travelled per frame (60hz) is 0.762 * 0.0275 = 0.020955 ~= 0.021

global struct vec3 mouse_acc;
global float look_sensitivity_x = 0.5f;
global float look_sensitivity_y = 0.5f;

global struct vec3 view_vel;
global struct vec3 view_acc;
global float player_acceleration = 4.0f;
global float sprint_factor = 2.0f;
global bool sprint = false;
global float friction_factor = 0.04f;

global float camera_slow_factor = 0.1f;
global bool camera_slow = false;

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
// Performance timing
global u64 perfs_frequency;
global u64 last_perfs;
global u64 last_cycles;

// FPS UI
global u64 fps_last_render;
global u64 fps_render_delta = 200000;  // 200 ms
global bool fps_render = false;

////////////////////////////////////////////////////////////////////////////////
// Key map and state
global u8 keystate_buffers[2][SDL_NUM_SCANCODES];

global u8* keys      = keystate_buffers[0];
global u8* keys_prev = keystate_buffers[1];

global u8 sdlk[RICOKEY_COUNT];

#define KEY_DOWN(key)     (keys[sdlk[key]])
#define KEY_PRESSED(key)  (keys[sdlk[key]] && !keys_prev[sdlk[key]])
#define KEY_RELEASED(key) (keys_prev[sdlk[key]] && !keys[sdlk[key]])

// Current state
global enum rico_state state;

// State change handlers
typedef int (*state_handler)();
state_handler state_handlers[STATE_COUNT] = { 0 };

int state_update(enum rico_state *_state)
{
    enum rico_error err;

    ///--------------------------------------------------------------------------
    //| Reset frame state
    ///--------------------------------------------------------------------------
    mouse_acc = VEC3_ZERO;

    ///--------------------------------------------------------------------------
    //| Handle input & state changes
    ///--------------------------------------------------------------------------
    enum rico_state start_state;
    bool state_changed = false;

    while (1)
    {
        start_state = state;
        err = state_handlers[state]();
        if (err)
            return err;

        if (state == start_state)
            break;

        state_changed = true;
    }
    *_state = state;

    if (state == STATE_ENGINE_SHUTDOWN)
        return SUCCESS;

    if (state_changed)
    {
        char buf[50] = { 0 };
        sprintf(buf, "State: %d %s", state, rico_state_string[state]);
        err = string_init("STR_STATE", STR_SLOT_EDIT_INFO, 0, 0,
                          COLOR_DARK_RED_HIGHLIGHT, 0, 0, buf);
        if (err) return err;
    }

    ///--------------------------------------------------------------------------
    //| Update time
    ///--------------------------------------------------------------------------
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
        int len = sprintf(buf, "%.f fps %.2f ms %.2f mcyc", fps, ms, mcyc);
        err = string_init("STR_FPS", STR_SLOT_FPS, SCREEN_W - (u32)(12.5 * len),
                          0, COLOR_DARK_RED_HIGHLIGHT, 0, 0, buf);
        if (err) return err;
        fps_last_render = last_perfs;
    }

    ///--------------------------------------------------------------------------
    //| Clear screen
    ///--------------------------------------------------------------------------
    //glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClearColor(0.46f, 0.70f, 1.0f, 1.0f);
    //glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ///--------------------------------------------------------------------------
    //| Update camera
    ///--------------------------------------------------------------------------

    //TODO: Only allow sprinting on the ground during normal play (X/Z)
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

    char buf[64] = { 0 };
    sprintf(buf, "Vel: %.1f %.1f %.1f", view_vel.x, view_vel.y, view_vel.z);
    err = string_init("STR_STATE", STR_SLOT_EDIT_INFO, 0, 0,
                      COLOR_DARK_RED_HIGHLIGHT, 0, 0, buf);
    if (err) return err;

    // Calculate delta position
    // dp' = 1/2at^2 + vt
    struct vec3 half_at_squared = view_acc;
    v3_scalef(&half_at_squared, 0.5f * (r32)dt * (r32)dt);
    struct vec3 vt = view_vel;
    v3_scalef(&vt, (r32)dt);

    struct vec3 delta_pos = half_at_squared;
    v3_add(&delta_pos, &vt);

    // Update position
    camera_translate_local(&cam_player, &delta_pos);

    // TODO: Smooth mouse look somehow
    if (mouse_acc.x != 0 || mouse_acc.y != 0)
    {
        struct vec3 delta = mouse_acc;
        delta.x *= look_sensitivity_x;
        delta.y *= look_sensitivity_y;
        camera_rotate(&cam_player, delta.x, delta.y);
    }

    camera_update(&cam_player);

    ///--------------------------------------------------------------------------
    //| Render
    ///--------------------------------------------------------------------------
    // glPolygonMode(GL_FRONT_AND_BACK, cam_player.fill_mode);
    glPolygonMode(GL_FRONT, cam_player.fill_mode);

    glref_update(dt);
    glref_render(&cam_player);
    camera_render(&cam_player);

    return err;
}

enum rico_state state_get()
{
    return state;
}

inline bool state_is_edit()
{
    return (state == STATE_EDIT_TRANSLATE ||
            state == STATE_EDIT_ROTATE    ||
            state == STATE_EDIT_SCALE     ||
            state == STATE_EDIT_TEXTURE   ||
            state == STATE_EDIT_MESH);
}

internal int save_file()
{
    enum rico_error err;

    struct rico_chunk *chunk = chunk_active();

    if (chunk->uid.uid == UID_NULL)
        return RICO_ERROR(ERR_CHUNK_NULL, "Failed to save NULL chunk");

    struct rico_file file;
    err = rico_file_open_write(&file, "chunks/cereal.bin",
                               RICO_FILE_VERSION_CURRENT);
    if (err) return err;

    err = rico_serialize(chunk, &file);
    rico_file_close(&file);

    // Save backup copy
    time_t rawtime = time(NULL);
    struct tm *tm = localtime(&rawtime);

    char backupFilename[128] = { 0 };
    snprintf(backupFilename, sizeof(backupFilename),
             "chunks/cereal_%04d%02d%02dT%02d%02d%02d.bin", 1900 + tm->tm_year,
             1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    struct rico_file backupFile;
    err = rico_file_open_write(&backupFile, backupFilename,
                               RICO_FILE_VERSION_CURRENT);
    if (err) return err;

    err = rico_serialize(chunk, &backupFile);
    rico_file_close(&backupFile);

    return err;
}

///----------------------------------------------------------------------
//| Camera forward ray v. scene
///----------------------------------------------------------------------
internal void select_first_obj()
{
    #define COLLIDE_COUNT 10
    u32 obj_collided[COLLIDE_COUNT] = { 0 };
    float dist[COLLIDE_COUNT] = { 0 };
    u32 idx_first = 0;
    struct ray cam_fwd;

    camera_fwd(&cam_fwd, &cam_player);
    u32 collided = object_collide_ray_type(OBJ_STATIC, &cam_fwd,
                                           COLLIDE_COUNT, obj_collided,
                                           dist, &idx_first);
    select_obj(obj_collided[idx_first]);

    // TODO: Loop through collided objects?
    UNUSED(collided);
    // if (collided > 0)
    // {
    //     printf("Colliding: ");
    //     for (int i = 0; i < COLLIDE_COUNT; ++i)
    //     {
    //         if (obj_collided[i] == 0) break;
    //         printf(" %d", obj_collided[i]);
    //     }
    //     printf("\n");
    // }
}

internal int sdl_poll(SDL_Event *event)
{
    int has_events = SDL_PollEvent(event);

    // Get keyboard state
    u8 *keys_tmp = keys_prev;
    keys_prev = keys;
    keys = keys_tmp;
    memcpy(keys, SDL_GetKeyboardState(0), SDL_NUM_SCANCODES);

    return has_events;
}

internal int handle_engine_events(SDL_Event event, bool *handled)
{
    enum rico_error err = SUCCESS;

    // [WINDOW CLOSE]: Exit application
    if (event.type == SDL_QUIT)
    {
        state = STATE_ENGINE_SHUTDOWN;
        *handled = true;
    }
    ////////////////////////////////////////////////////////////////////////////
    // TODO: Seriously, how do I check for SDL resize window events?
    ////////////////////////////////////////////////////////////////////////////
    /*
    // [WINDOW RESIZE]: Resize OpenGL viewport
    else if (event.type == SDL_WINDOWEVENT_SIZE)
    {
        glViewport(0, 0, event.window.event.size.width,
                   event.window.event.size.height);
        *handled = true;
    }
    */
    // [MOUSE MOVE]: Record relative position of mouse cursor
    else if (event.type == SDL_MOUSEMOTION)
    {
        if (mouse_lock)
        {
            mouse_acc.x = (float)event.motion.xrel;
            // if (mouse_acc.x >  1) mouse_acc.x =  1;
            // if (mouse_acc.x < -1) mouse_acc.x = -1;

            mouse_acc.y = (float)event.motion.yrel;
            // if (mouse_acc.y >  1) mouse_acc.y =  1;
            // if (mouse_acc.y < -1) mouse_acc.y = -1;
        }
        *handled = true;
    }
    else if (event.type == SDL_KEYDOWN && !event.key.repeat)
    {
        if (event.key.keysym.mod & KMOD_CTRL &&
            event.key.keysym.mod & KMOD_SHIFT) {}
        else if (event.key.keysym.mod & KMOD_CTRL &&
                 event.key.keysym.mod & KMOD_ALT) {}
        else if (event.key.keysym.mod & KMOD_SHIFT)
        {
            // [`]: DEBUG: Show a test string for 3 seconds
            if (keys[SDL_SCANCODE_GRAVE])
            {
                err = string_init("STR_EDIT_MODE", STR_SLOT_DYNAMIC, 0, 0,
                                  COLOR_GREEN, 3000, 0,
                                  "Blah blah testing some stuff.\n3333");
                if (err) return err;
                *handled = true;
            }
        }
        else if (event.key.keysym.mod & KMOD_CTRL)
        {
            // [Ctrl-S]: Save chunk
            if (keys[SDL_SCANCODE_S])
            {
                save_file();
                *handled = true;
            }
        }
        else if (event.key.keysym.mod & KMOD_ALT)
        {}
        // [2]: Toggle FPS counter
        else if (keys[SDL_SCANCODE_2])
        {
            fps_render = !fps_render;
            if (!fps_render)
                string_free(STR_SLOT_FPS);
            *handled = true;
        }
        // [Esc]: Save and exit
        else if (keys[SDL_SCANCODE_ESCAPE])
        {
            err = string_init("STR_CONFIRM_QUIT", STR_SLOT_MENU_QUIT, 600,
                              400, COLOR_GREEN, 0, 0,
                              "Are you sure you want to quit?\n" \
                              "                              \n" \
                              "      Yes (Y) or No (N)       ");
            if (err) return err;
            state = STATE_MENU_QUIT;
            *handled = true;
        }
        // [L]: Toggle scene lighting
        else if (keys[SDL_SCANCODE_L])
        {
            // TODO: Pretty sure this is broken
            // TODO: Use this to change shader program on render
            enable_lighting = !enable_lighting;
            *handled = true;
        }
        // [M]: Toggle mouse lock-to-window
        else if (keys[SDL_SCANCODE_M])
        {
            mouse_lock = !mouse_lock;
            SDL_SetRelativeMouseMode(mouse_lock);
            *handled = true;
        }
#if DEBUG
        // [P]: DEBUG: Force breakpoint
        else if (keys[SDL_SCANCODE_p])
        {
             sdl_triggerbreakpoint();
             *handled = true;
        }
#endif
    }

    return err;
}

internal int handle_camera_events(SDL_Event event, bool *handled)
{
    enum rico_error err = SUCCESS;

    view_acc = VEC3_ZERO;

    if KEY_DOWN(RICOKEY_UP)       view_acc.y += 1.0f;
    if KEY_DOWN(RICOKEY_DOWN)     view_acc.y -= 1.0f;
    if KEY_DOWN(RICOKEY_RIGHT)    view_acc.x += 1.0f;
    if KEY_DOWN(RICOKEY_LEFT)     view_acc.x -= 1.0f;
    if KEY_DOWN(RICOKEY_FORWARD)  view_acc.z += 1.0f;
    if KEY_DOWN(RICOKEY_BACKWARD) view_acc.z -= 1.0f;

    sprint = KEY_DOWN(RICOKEY_SPRINT);

    if KEY_PRESSED(RICOKEY_CAMERA_SLOW)  camera_slow = !camera_slow;
    if KEY_PRESSED(RICOKEY_CAMERA_RESET) camera_reset(&cam_player);
    if KEY_PRESSED(RICOKEY_CAMERA_LOCK)  cam_player.locked = !cam_player.locked;
    if KEY_PRESSED(RICOKEY_CAMERA_WIREFRAME) cam_player.fill_mode =
        (cam_player.fill_mode == GL_FILL) ? GL_LINE : GL_FILL;

    // TODO: Fix diagonal movement
    //v3_normalize(&delta);
    //view_acc = delta;

    return err;
}

internal int state_play_explore()
{
    enum rico_error err = SUCCESS;

    SDL_Event event;
    while (state == STATE_PLAY_EXPLORE && sdl_poll(&event))
    {
        bool handled = 0;

        err = handle_engine_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        err = handle_camera_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}

            // [`]: Enter edit mode
            else if (keys[SDL_SCANCODE_GRAVE]) // backtick
            {
                state = STATE_EDIT_TRANSLATE;
                selected_print();
            }
        }
    }

    return err;
}

internal int handle_edit_events(SDL_Event event, bool *handled)
{
    RICO_ASSERT(state_is_edit());

    enum rico_error err;

    err = handle_engine_events(event, handled);
    if (err)      return err;
    if (*handled) return SUCCESS;

    err = handle_camera_events(event, handled);
    if (err)      return err;
    if (*handled) return SUCCESS;

    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        // [Mouse Left Click]: Raycast object selection
        if (event.button.button == SDL_BUTTON_LEFT)
        {
            select_first_obj();
            *handled = true;
        }
    }
    else if (event.type == SDL_KEYDOWN && !event.key.repeat)
    {
        if (event.key.keysym.mod & KMOD_CTRL &&
            event.key.keysym.mod & KMOD_SHIFT) {
        }
        else if (event.key.keysym.mod & KMOD_CTRL &&
                 event.key.keysym.mod & KMOD_ALT) {
        }
        else if (event.key.keysym.mod & KMOD_SHIFT)
        {
            // [Shift-Tab]: Cycle select through objects (in reverse)
            if (keys[SDL_SCANCODE_TAB])
            {
                select_prev_obj();
                *handled = true;
            }
            // [Shift-B]: Recalculate bounding boxes of all objects
            else if (keys[SDL_SCANCODE_B])
            {
                recalculate_all_bbox();
                *handled = true;
            }
        }
        else if (event.key.keysym.mod & KMOD_CTRL)
        {
            // [Ctrl-D]: Duplicate selected object
            if (keys[SDL_SCANCODE_D])
            {
                selected_duplicate();
                *handled = true;
            }
        }
        else if (event.key.keysym.mod & KMOD_ALT) {}

        // [`]: Exit edit mode
        else if (keys[SDL_SCANCODE_GRAVE])
        {
            string_free(STR_SLOT_SELECTED_OBJ);
            state = STATE_PLAY_EXPLORE;
            *handled = true;
        }
        // [Numpad 0]: Select next edit mode
        else if (keys[SDL_SCANCODE_KP_0])
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
            *handled = true;
        }
        // [Numpad .]: Select previous edit mode
        else if (keys[SDL_SCANCODE_KP_PERIOD])
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
            *handled = true;
        }
        // [Tab]: Cycle select through objects
        else if (keys[SDL_SCANCODE_TAB])
        {
            select_next_obj();
            *handled = true;
        }
        // [Ins]: Create new object
        else if (keys[SDL_SCANCODE_INSERT])
        {
            create_obj();
            *handled = true;
        }
        // [Del]: Delete selected object
        else if (keys[SDL_SCANCODE_DELETE])
        {
            selected_delete();
            *handled = true;
        }
    }

    return SUCCESS;
}

internal int state_edit_translate()
{
    enum rico_error err = SUCCESS;

    struct vec3 translate = VEC3_ZERO;
    bool translate_reset = false;
    bool trans_delta_changed = false;

    SDL_Event event;
    while (state == STATE_EDIT_TRANSLATE && sdl_poll(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}
            // [0]: Reset selected object's translation
            else if (keys[SDL_SCANCODE_0])
                translate_reset = true;
            // [Up Arrow]: Translate selected object up
            else if (keys[SDL_SCANCODE_UP])
                translate.y += trans_delta;
            // [Down Arrow]: Translate selected object down
            else if (keys[SDL_SCANCODE_DOWN])
                translate.y -= trans_delta;
            // [Left Arrow]: Translate selected object west
            else if (keys[SDL_SCANCODE_LEFT])
                translate.x -= trans_delta;
            // [Right Arrow]: Translate selected object east
            else if (keys[SDL_SCANCODE_RIGHT])
                translate.x += trans_delta;
            // [Page Up]: Translate selected object north
            else if (keys[SDL_SCANCODE_PAGEUP])
                translate.z -= trans_delta;
            // [Page Down]: Translate selected object south
            else if (keys[SDL_SCANCODE_PAGEDOWN])
                translate.z += trans_delta;
            // [Numpad +]: Increase translation delta
            else if (keys[SDL_SCANCODE_KP_PLUS])
            {
                if (trans_delta < TRANS_DELTA_MAX)
                {
                    trans_delta *= 10.0f;
                    if (trans_delta > TRANS_DELTA_MAX)
                        trans_delta = TRANS_DELTA_MAX;

                    trans_delta_changed = true;
                }
            }
            // [Numpad -]: Decrease translation delta
            else if (keys[SDL_SCANCODE_KP_MINUS])
            {
                if (trans_delta > TRANS_DELTA_MIN)
                {
                    trans_delta /= 10.0f;
                    if (trans_delta < TRANS_DELTA_MIN)
                        trans_delta = TRANS_DELTA_MIN;

                    trans_delta_changed = true;
                }
            }
        }
    }

    if (translate_reset)
    {
        selected_translate(&cam_player, &VEC3_ZERO);
    }
    else if (!v3_equals(&translate, &VEC3_ZERO))
    {
        selected_translate(&cam_player, &translate);
    }

    if (trans_delta_changed)
    {
        char buf[50] = { 0 };
        sprintf(buf, "Trans Delta: %f", trans_delta);
        err = string_init("STR_EDIT_TRANS_DELTA", STR_SLOT_EDIT_INFO, 0, 0,
                          COLOR_DARK_BLUE_HIGHLIGHT, 1000, 0, buf);
        if (err) return err;
    }

    return err;
}

internal int state_edit_rotate()
{
    enum rico_error err = SUCCESS;

    struct vec3 rotate = VEC3_ZERO;
    bool rotate_reset = false;
    bool rot_delta_changed = false;

    SDL_Event event;
    while (state == STATE_EDIT_ROTATE && sdl_poll(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}
            // [7]: Toggle heptagonal rotations
            else if (keys[SDL_SCANCODE_7])
            {
                if (rot_delta == ROT_DELTA_DEFAULT)
                    rot_delta = (float)M_SEVENTH_DEG;
                else
                    rot_delta = ROT_DELTA_DEFAULT;
            }
            // [0]: Reset selected object's rotation
            else if (keys[SDL_SCANCODE_0])
                rotate_reset = true;
            // [Up Arrow]: Rotate selected object up
            else if (keys[SDL_SCANCODE_UP])
                rotate.x -= rot_delta;
            // [Down Arrow]: Rotate selected object down
            else if (keys[SDL_SCANCODE_DOWN])
                rotate.x += rot_delta;
            // [Left Arrow]: Rotate selected object west
            else if (keys[SDL_SCANCODE_LEFT])
                rotate.y -= rot_delta;
            // [Right Arrow]: Rotate selected object east
            else if (keys[SDL_SCANCODE_RIGHT])
                rotate.y += rot_delta;
            // [Page Up]: Rotate selected object north
            else if (keys[SDL_SCANCODE_PAGEUP])
                rotate.z -= rot_delta;
            // [Page Down]: Rotate selected object south
            else if (keys[SDL_SCANCODE_PAGEDOWN])
                rotate.z += rot_delta;
            // [Numpad +]: Increase rotation delta
            else if (keys[SDL_SCANCODE_KP_PLUS])
            {
                if (rot_delta < ROT_DELTA_MAX)
                {
                    rot_delta += (rot_delta < 5.0f) ? 1.0f : 5.0f;
                    if (rot_delta > ROT_DELTA_MAX)
                        rot_delta = ROT_DELTA_MAX;

                    rot_delta_changed = true;
                }
            }
            // [Numpad -]: Decrease rotation delta
            else if (keys[SDL_SCANCODE_KP_MINUS])
            {
                if (rot_delta > ROT_DELTA_MIN)
                {
                    rot_delta -= (rot_delta > 5.0f) ? 5.0f : 1.0f;
                    if (rot_delta < ROT_DELTA_MIN)
                        rot_delta = ROT_DELTA_MIN;

                    rot_delta_changed = true;
                }
            }
        }
    }

    if (rotate_reset)
    {
        selected_rotate(&VEC3_ZERO);
    }
    else if (!v3_equals(&rotate, &VEC3_ZERO))
    {
        selected_rotate(&rotate);
    }

    if (rot_delta_changed)
    {
        char buf[50] = { 0 };
        sprintf(buf, "Rot Delta: %f", rot_delta);
        err = string_init("STR_EDIT_ROT_DELTA", STR_SLOT_EDIT_INFO, 0, 0,
                          COLOR_DARK_BLUE_HIGHLIGHT, 1000, 0, buf);
        if (err) return err;
    }

    return err;
}

internal int state_edit_scale()
{
    enum rico_error err = SUCCESS;

    struct vec3 scale = VEC3_ZERO;
    bool scale_reset = false;
    bool scale_delta_changed = false;

    SDL_Event event;
    while (state == STATE_EDIT_SCALE && sdl_poll(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}
            // [0]: Reset selected object's scale
            else if (keys[SDL_SCANCODE_0])
                scale_reset = true;
            // [Up Arrow]: Scale selected object up
            else if (keys[SDL_SCANCODE_UP])
                scale.y += scale_delta;
            // [Down Arrow]: Scale selected object down
            else if (keys[SDL_SCANCODE_DOWN])
                scale.y -= scale_delta;
            // [Left Arrow]: Scale selected object west
            else if (keys[SDL_SCANCODE_LEFT])
                scale.x -= scale_delta;
            // [Right Arrow]: Scale selected object east
            else if (keys[SDL_SCANCODE_RIGHT])
                scale.x += scale_delta;
            // [Page Up]: Scale selected object north
            else if (keys[SDL_SCANCODE_PAGEUP])
                scale.z += scale_delta;
            // [Page Down]: Scale selected object south
            else if (keys[SDL_SCANCODE_PAGEDOWN])
                scale.z -= scale_delta;
            // [Numpad +]: Increase scale delta
            else if (keys[SDL_SCANCODE_KP_PLUS])
            {
                if (scale_delta < SCALE_DELTA_MAX)
                {
                    scale_delta += (scale_delta < 1.0f) ? 0.1f : 1.0f;
                    if (scale_delta > SCALE_DELTA_MAX)
                        scale_delta = SCALE_DELTA_MAX;

                    scale_delta_changed = true;
                }
            }
            // [Numpad -]: Decrease scale delta
            else if (keys[SDL_SCANCODE_KP_MINUS])
            {
                if (scale_delta > SCALE_DELTA_MIN)
                {
                    scale_delta -= (scale_delta > 1.0f) ? 1.0f : 0.1f;
                    if (scale_delta < SCALE_DELTA_MIN)
                        scale_delta = SCALE_DELTA_MIN;

                    scale_delta_changed = true;
                }
            }
        }
    }

    if (scale_reset)
    {
        selected_scale(&VEC3_ZERO);
    }
    else if (!v3_equals(&scale, &VEC3_ZERO))
    {
        selected_scale(&scale);
    }

    if (scale_delta_changed)
    {
        char buf[50] = { 0 };
        sprintf(buf, "Scale Delta: %f", scale_delta);
        err = string_init("STR_EDIT_SCALE_DELTA", STR_SLOT_EDIT_INFO, 0, 0,
                          COLOR_DARK_BLUE_HIGHLIGHT, 1000, 0, buf);
        if (err) return err;
    }

    return err;
}

internal int state_edit_texture()
{
    enum rico_error err = SUCCESS;

    SDL_Event event;
    while (state == STATE_EDIT_TEXTURE && sdl_poll(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}
        }
    }

    return err;
}

internal int state_edit_mesh()
{
    enum rico_error err = SUCCESS;

    SDL_Event event;
    while (state == STATE_EDIT_MESH && sdl_poll(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}

            // [Left Arrow]: Cycle selected object's mesh (in reverse)
            else if (keys[SDL_SCANCODE_LEFT])
                selected_mesh_prev();
            // [Right Arrow]: Cycle selected object's mesh
            else if (keys[SDL_SCANCODE_RIGHT])
                selected_mesh_next();
            // [B]: Recalculate bounding box based on current mesh
            else if (keys[SDL_SCANCODE_B])
                selected_bbox_reset();
        }
    }

    return err;
}

internal int state_menu_quit()
{
    enum rico_error err = SUCCESS;

    SDL_Event event;
    while (state == STATE_MENU_QUIT && sdl_poll(&event))
    {
        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {
            }
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {
            }
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}

            // [Y] / [Return]: Confirm: Save and exit
            else if (keys[SDL_SCANCODE_Y] || keys[SDL_SCANCODE_RETURN])
            {
                string_free(STR_SLOT_MENU_QUIT);
                save_file();
                state = STATE_ENGINE_SHUTDOWN;
            }
            // [N] / [Escape]: Abort: Return to play mode
            else if (keys[SDL_SCANCODE_N] || keys[SDL_SCANCODE_ESCAPE])
            {
                string_free(STR_SLOT_MENU_QUIT);
                state = STATE_PLAY_EXPLORE;
                selected_print();
            }
        }
    }

    return err;
}

internal int state_text_input()
{
    return SUCCESS;
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
    // Custom serialiers
    rico_cereals[RICO_UID_CHUNK].save[0] = &chunk_serialize_0;
    rico_cereals[RICO_UID_CHUNK].load[0] = &chunk_deserialize_0;

    rico_cereals[RICO_UID_POOL].save[0] = &pool_serialize_0;
    rico_cereals[RICO_UID_POOL].load[0] = &pool_deserialize_0;

    rico_cereals[RICO_UID_OBJECT].save[0] = &object_serialize_0;
    rico_cereals[RICO_UID_OBJECT].load[0] = &object_deserialize_0;

    rico_cereals[RICO_UID_MATERIAL].save[0] = &material_serialize_0;
    rico_cereals[RICO_UID_MATERIAL].load[0] = &material_deserialize_0;

    rico_cereals[RICO_UID_BBOX].save[0] = &bbox_serialize_0;
    rico_cereals[RICO_UID_BBOX].load[0] = &bbox_deserialize_0;
}

internal int rico_init_chunks()
{
    enum rico_error err;

    // TODO: Create a hard-coded test chunk that gets loaded if the real chunk
    //       can't be loaded from the save file.

    err = chunk_init("RICO_CHUNK_DEFAULT",
                     RICO_STRING_POOL_SIZE,
                     RICO_FONT_POOL_SIZE,
                     RICO_TEXTURE_POOL_SIZE,
                     RICO_MATERIAL_POOL_SIZE,
                     RICO_MESH_POOL_SIZE,
                     RICO_OBJECT_POOL_SIZE,
                     &RICO_DEFAULT_CHUNK);
    if (err) return err;

    chunk_active_set(RICO_DEFAULT_CHUNK);

    return err;
}

internal int rico_init_fonts()
{
    enum rico_error err;

    // TODO: Use fixed slots to allocate default resources
    err = font_init("font/courier_new.bff", &RICO_DEFAULT_FONT);
    return err;
}

internal int rico_init_textures()
{
    enum rico_error err;

    // TODO: Use fixed slots to allocate default resources
    err = texture_load_file("TEXTURE_DEFAULT_DIFF", GL_TEXTURE_2D,
                            "texture/basic_diff.tga", 32,
                            &RICO_DEFAULT_TEXTURE_DIFF);
    if (err) return err;

    err = texture_load_file("TEXTURE_DEFAULT_SPEC", GL_TEXTURE_2D,
                            "texture/basic_spec.tga", 32,
                            &RICO_DEFAULT_TEXTURE_SPEC);
    return err;
}

internal int rico_init_materials()
{
    enum rico_error err;

    // TODO: Use fixed slots to allocate default resources
    err = material_init("MATERIAL_DEFAULT", RICO_DEFAULT_TEXTURE_DIFF,
                        RICO_DEFAULT_TEXTURE_SPEC, 0.5f,
                        &RICO_DEFAULT_MATERIAL);
    return err;
}

internal int rico_init_meshes()
{
    enum rico_error err;

    //--------------------------------------------------------------------------
    // Create default mesh (white rect)
    //--------------------------------------------------------------------------
    #define vert_count 8
    #define element_count 36
    const struct mesh_vertex vertices[vert_count] = {
        {
            { -1.0f, -1.0f, 0.01f },   //Position
            { 1.0f, 1.0f, 1.0f, 1.0f }, //Color
            { 0.0f, 0.0f, 1.0f },       //Normal
            { 0.0f, 0.0f }              //UV-coords
        },
        {
            { 1.0f, -1.0f, 0.01f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        {
            { 1.0f, 1.0f, 0.01f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f }
        },
        {
            { -1.0f, 1.0f, 0.01f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f }
        },
        {
            { -1.0f, -1.0f, -0.01f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f }
        },
        {
            { 1.0f, -1.0f, -0.01f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        {
            { 1.0f, 1.0f, -0.01f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f }
        },
        {
            { -1.0f, 1.0f, -0.01f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f }
        }
    };
    const GLuint elements[element_count] = {
        0, 1, 2, 2, 3, 0,
        4, 0, 3, 3, 7, 4,
        5, 4, 7, 7, 6, 5,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };

    // TODO: Use fixed slot for default mesh (do this for all other defaults
    //       as well.
    err = mesh_load(&RICO_DEFAULT_MESH, "MESH_DEFAULT", MESH_OBJ_WORLD,
                    vert_count, vertices, element_count, elements,
                    GL_STATIC_DRAW);
    return err;
}

internal int load_mesh_files()
{
    enum rico_error err;

    u32 ticks = SDL_GetTicks();

    err = load_obj_file("mesh/sphere.ric");
    if (err) return err;

    const char *sphere_mesh_name = "Sphere";
    PRIM_SPHERE_MESH = mesh_request_by_name(sphere_mesh_name);
    if (!PRIM_SPHERE_MESH)
        return RICO_ERROR(ERR_MESH_INVALID_NAME, "Invalid mesh name: %s",
                          sphere_mesh_name);

    // err = load_obj_file("mesh/conference.ric");
    // if (err) return err;

    err = load_obj_file("mesh/spawn.ric");
    if (err) return err;

    err = load_obj_file("mesh/door.ric");
    if (err) return err;

    err = load_obj_file("mesh/welcome_floor.ric");
    if (err) return err;

    err = load_obj_file("mesh/wall_cornertest.ric");
    if (err) return err;

    err = load_obj_file("mesh/grass.ric");
    if (err) return err;

    u32 ticks2 = SDL_GetTicks();
    printf("[PERF][mesh] Meshes loaded in: %d ticks\n", ticks2 - ticks);

    return err;
}

internal int init_hardcoded_test_chunk()
{
    enum rico_error err;

    //--------------------------------------------------------------------------
    // Create materials
    //--------------------------------------------------------------------------
    u32 material_rock;
    err = material_init("Rock", tex_rock, RICO_DEFAULT_TEXTURE_SPEC, 0.5f,
                        &material_rock);
    if (err) return err;

    //--------------------------------------------------------------------------
    // Create world objects
    //--------------------------------------------------------------------------

    // Ground
    u32 obj_ground;
    err = object_create(&obj_ground, "Ground", OBJ_STATIC, RICO_DEFAULT_MESH,
                        material_rock, NULL, true);
    if (err) return err;
    object_rot_x(obj_ground, -90.0f);
    object_scale(obj_ground, &(struct vec3) { 64.0f, 64.0f, 1.0f });

    // TEST: Create test object for each mesh / primitive
    {
        u32 arr_objects[RICO_MESH_POOL_SIZE] = { 0 };
        u32 i = 0;

        /*
        // Cleanup: Could use mesh_pool_get_unsafe(), but what's really the
        //          the point of this code?
        // Create test object for each loaded mesh
        for (; i < mesh_count; i++)
        {
        err = object_create(&arr_objects[i], mesh_name(meshes[i]),
        OBJ_STATIC, meshes[i], RICO_MATERIAL_DEFAULT,
        mesh_bbox(meshes[i]), true);
        if (err) return err;

        // HACK: Don't z-fight ground plane
        object_trans_set(arr_objects[i],
        &(struct vec3) { 0.0f, EPSILON, 0.0f });

        // HACK: Scale scene 1/10 (for conference room test)
        // object_scale_set(arr_objects[i],
        //                  &(struct vec3) { 0.1f, 0.1f, 0.1f });
        }
        */

        // Create test object for each primitive
        err = object_create(&arr_objects[i], mesh_name(PRIM_SPHERE_MESH),
                            OBJ_STATIC, PRIM_SPHERE_MESH,
                            RICO_DEFAULT_MATERIAL, mesh_bbox(PRIM_SPHERE_MESH),
                            true);
        i++;
        if (err) return err;

        // HACK: Don't z-fight ground plane
        //object_trans_set(arr_objects[i],
        //                 &(struct vec3) { 0.0f, EPSILON, 0.0f });
    }

    //--------------------------------------------------------------------------
    // Save manual chunk
    //--------------------------------------------------------------------------
    //err = chunk_init("my_first_chunk", 1,
    //                 RICO_STRING_POOL_SIZE,
    //                 RICO_FONT_POOL_SIZE,
    //                 RICO_TEXTURE_POOL_SIZE,
    //                 RICO_MATERIAL_POOL_SIZE,
    //                 RICO_MESH_POOL_SIZE,
    //                 RICO_OBJECT_POOL_SIZE,
    //                 &chunk_home);
    //if (err) return err;

    /*   struct rico_file file;
    err = rico_file_open_write(&file, "chunks/cereal.bin",
    RICO_FILE_VERSION_CURRENT);
    if (err) return err;

    err = rico_serialize(chunk_home, &file);
    rico_file_close(&file);*/

    return err;
}

internal int rico_init()
{
    enum rico_error err;

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing serializers\n");
    printf("----------------------------------------------------------\n");
    rico_init_cereal();

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

    if (reset_game_world)
    {
        printf("----------------------------------------------------------\n");
        printf("[MAIN][init] Initializing chunks\n");
        printf("----------------------------------------------------------\n");
        err = rico_init_chunks();
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

        // TODO: Load mesh data directly into OpenGL, independant of creating an
        //       accompanying rico_mesh object. Fixup VAO ids by having a
        //       mesh_name lookup table. Load meshes into VRAM as necessary if
        //       they aren't found in the lookup table.
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
        printf("[MAIN][init] Initializing game world\n");
        printf("----------------------------------------------------------\n");
        err = init_glref();
        if (err) return err;
    
        printf("Loading hard-coded test chunk\n");
        err = init_hardcoded_test_chunk();
        if (err) return err;

        chunk_active_set(RICO_DEFAULT_CHUNK);
    }
    else
    {
        printf("----------------------------------------------------------\n");
        printf("[MAIN][init] Initializing chunks\n");
        printf("----------------------------------------------------------\n");
        struct rico_file file;
        err = rico_file_open_read(&file, "chunks/cereal.bin");
        if (err) return err;

        struct rico_chunk chunk = { 0 };
        uid_init(&chunk.uid, RICO_UID_CHUNK, "CHUNK_LOADING", false);
        struct rico_chunk *chunk_ptr = &chunk;
        err = rico_deserialize(&chunk_ptr, &file);
        rico_file_close(&file);
        if (err) return err;

        // TODO: Use fixed slots, or find by name and load if don't exist?
        RICO_DEFAULT_FONT = 1;
        RICO_DEFAULT_TEXTURE_DIFF = 1;
        RICO_DEFAULT_TEXTURE_SPEC = 2;
        RICO_DEFAULT_MATERIAL = 1;
        RICO_DEFAULT_MESH = 1;
        //RICO_DEFAULT_OBJECT = 1;

        chunk_active_set(chunk_ptr);
    }

    printf("----------------------------------------------------------\n");
    printf("[MAIN][init] Initializing camera\n");
    printf("----------------------------------------------------------\n");
    camera_reset(&cam_player);
    return err;
}

internal int state_engine_init()
{
    enum rico_error err;

    printf("----------------------------------------------------------\n");
    printf("#         ______            _______        _               #\n");
    printf("#         |  __ \\ O        |__   __|      | |              #\n");
    printf("#         | |__| |_  ___ ___  | | ___  ___| |__            #\n");
    printf("#         |  _  /| |/ __/ _ \\ | |/ _ \\/ __| '_ \\           #\n");
    printf("#         | | \\ \\| | |_| (_) || |  __/ |__| | | |          #\n");
    printf("#         |_|  \\_\\_|\\___\\___/ |_|\\___|\\___|_| |_|          #\n");
    printf("#                                                          #\n");
    printf("#               Copyright 2017 Dan Bechard                 #\n");
    printf("----------------------------------------------------------\n");

    // TODO: Where does this belong? Needs access to "mouse_lock".
    SDL_SetRelativeMouseMode(mouse_lock);

    perfs_frequency = SDL_GetPerformanceFrequency();
    last_perfs = SDL_GetPerformanceCounter();
    last_cycles = __rdtsc();
    fps_last_render = last_perfs;
    view_vel = VEC3_ZERO;
    view_acc = VEC3_ZERO;

    // TODO: Load from config file?
    // Initialize key map
    sdlk[RICOKEY_RIGHT]    = SDL_SCANCODE_D;
    sdlk[RICOKEY_LEFT]     = SDL_SCANCODE_A;
    sdlk[RICOKEY_UP]       = SDL_SCANCODE_E;
    sdlk[RICOKEY_DOWN]     = SDL_SCANCODE_Q;
    sdlk[RICOKEY_FORWARD]  = SDL_SCANCODE_W;
    sdlk[RICOKEY_BACKWARD] = SDL_SCANCODE_S;
    sdlk[RICOKEY_SPRINT]   = SDL_SCANCODE_LSHIFT;

    sdlk[RICOKEY_CAMERA_SLOW]       = SDL_SCANCODE_R;
    sdlk[RICOKEY_CAMERA_RESET]      = SDL_SCANCODE_F;
    sdlk[RICOKEY_CAMERA_LOCK]       = SDL_SCANCODE_C;
    sdlk[RICOKEY_CAMERA_WIREFRAME]  = SDL_SCANCODE_1;

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
    state_handlers[STATE_ENGINE_INIT]     = &state_engine_init;
    state_handlers[STATE_PLAY_EXPLORE]    = &state_play_explore;
    state_handlers[STATE_EDIT_TRANSLATE]  = &state_edit_translate;
    state_handlers[STATE_EDIT_ROTATE]     = &state_edit_rotate;
    state_handlers[STATE_EDIT_SCALE]      = &state_edit_scale;
    state_handlers[STATE_EDIT_TEXTURE]    = &state_edit_texture;
    state_handlers[STATE_EDIT_MESH]       = &state_edit_mesh;
    state_handlers[STATE_MENU_QUIT]       = &state_menu_quit;
    state_handlers[STATE_TEXT_INPUT]      = &state_text_input;
    state_handlers[STATE_ENGINE_SHUTDOWN] = &state_engine_shutdown;
    state = STATE_ENGINE_INIT;
}
