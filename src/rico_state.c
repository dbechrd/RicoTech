#include "rico_state.h"
#include "const.h"
#include "geom.h"
#include "rico_string.h"
#include "rico_font.h"
#include "camera.h"
#include "glref.h"
#include "primitives.h"
#include "rico_object.h"
#include "rico_chunk.h"
#include "rico_material.h"
#include "load_object.h"
#include "GL/gl3w.h"
#include "SDL/SDL.h"
#include <time.h>

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
    RICOKEY_COUNT
};

////////////////////////////////////////////////////////////////////////////////
static const bool reset_game_world = false;
static struct rico_chunk RICO_CHUNK_DEFAULT;

////////////////////////////////////////////////////////////////////////////////
//Human walk speed empirically found to be 33 steps in 20 seconds. That
//is approximately 1.65 steps per second. At 60 fps, that is 0.0275 steps
//per frame. Typical walking stride is ~0.762 meters (30 inches). Distance
//travelled per frame (60hz) is 0.762 * 0.0275 = 0.020955 ~= 0.021

//TODO: This assumes 1/60th second (60fps), this should be 1.65 * dt.
//GLfloat view_acc_delta = 0.001f;
//GLfloat view_acc_delta = 1.65f;

////////////////////////////////////////////////
// Before delta change
////////////////////////////////////////////////
// static struct vec3 view_vel;
// static GLfloat view_vel_max = 8.0f;
// static GLfloat view_vel_max_sprint = 16.0f;

// static struct vec3 view_acc;
// static GLfloat view_acc_delta = 0.05f;
// static GLfloat sprint_factor = 2.0f;
////////////////////////////////////////////////
static float look_sensitivity_x = 0.5f;
static float look_sensitivity_y = 0.5f;
static struct vec3 mouse_acc;

static struct vec3 view_vel;
static float view_vel_max = 2.5f; //5.0f;
static float view_vel_max_sprint = 10.0f;
static float view_vel_max_snail = 0.05f;

static struct vec3 view_acc;
static float sprint_factor = 2.0f;

// static struct vec3 view_rot_vel;
// static float view_rot_vel_max = 300.0f;

// static struct vec3 view_rot_acc;
// static float view_rot_acc_max = 50.0f;
////////////////////////////////////////////////

#define TRANS_DELTA_MIN 0.01f
#define TRANS_DELTA_MAX 10.0f
static float trans_delta = 1.0f;

#define ROT_DELTA_MIN 1.0f
#define ROT_DELTA_MAX 90.0f
#define ROT_DELTA_DEFAULT 5.0f
static float rot_delta = ROT_DELTA_DEFAULT;

#define SCALE_DELTA_MIN 0.1f
#define SCALE_DELTA_MAX 5.0f
static float scale_delta = 1.0f;

static bool mouse_lock = true;
static bool sprint = false;
static bool snail = false;
static bool enable_lighting = true;

// Performance timing
static u64 perfs_frequency;
static u64 last_perfs;
static u64 last_cycles;

// FPS UI
static u64 fps_last_render;
static u64 fps_render_delta = 200000;  // 200 ms
static bool fps_render = false;

// Key map and state
static s32 sdlk[RICOKEY_COUNT];
static bool pressed[RICOKEY_COUNT] = { false };

// Current state
static enum rico_state state;

// State change handlers
typedef int (*state_handler)();
state_handler state_handlers[STATE_COUNT] = { 0 };

int state_update(enum rico_state *_state)
{
    enum rico_error err;

    ////////////////////////////////////////////////////////////////////////
    // Reset frame state
    ////////////////////////////////////////////////////////////////////////
    mouse_acc = VEC3_ZERO;

    ////////////////////////////////////////////////////////////////////////
    // Update time
    ////////////////////////////////////////////////////////////////////////
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
                          0, COLOR_DARK_RED_HIGHLIGHT, 0, RICO_FONT_DEFAULT,
                          buf);
        if (err) return err;
        fps_last_render = last_perfs;
    }

    ////////////////////////////////////////////////////////////////////////
    // Handle input & state changes
    ////////////////////////////////////////////////////////////////////////
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
                          COLOR_DARK_RED_HIGHLIGHT, 0, RICO_FONT_DEFAULT, buf);
        if (err) return err;
    }

    ////////////////////////////////////////////////////////////////////////
    // Clear screen
    ////////////////////////////////////////////////////////////////////////
    //glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClearColor(0.46f, 0.70f, 1.0f, 1.0f);
    //glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ////////////////////////////////////////////////////////////////////////
    // Update camera
    ////////////////////////////////////////////////////////////////////////
    float mag_acc = vec3_length(&view_acc);
    if (mag_acc < VEC3_EPSILON)
    {
        mag_acc = 0;
        view_acc = VEC3_ZERO;
    }
    else
    {
        struct vec3 delta = view_acc;
        if (sprint)
        {
            //TODO: Only allow sprinting on the ground during normal play (X/Z)
            vec3_scalef(&delta, sprint_factor);
        }
        // vec3_scalef(&delta, dt);

        vec3_add(&view_vel, &delta);
    }

    float mag_vel = vec3_length(&view_vel);
    if (mag_vel < VEC3_EPSILON)
    {
        mag_vel = 0;
        view_vel = VEC3_ZERO;
    }
    else
    {
        if (mag_acc == 0)
        {
            vec3_scalef(&view_vel, 0.75f);
        }

        float vel_max = (sprint) ? view_vel_max_sprint : view_vel_max;
        vel_max = (snail) ? view_vel_max_snail : vel_max;
        if (mag_vel > vel_max)
        {
            vec3_normalize(&view_vel);
            vec3_scalef(&view_vel, vel_max);
        }

        struct vec3 delta = view_vel;
        vec3_scalef(&delta, (r32)dt);

        camera_translate_local(&cam_player, &delta);
    }

    // TODO: Smooth mouse look somehow
    if (mouse_acc.x != 0 || mouse_acc.y != 0)
    {
        struct vec3 delta = mouse_acc;
        delta.x *= look_sensitivity_x;
        delta.y *= look_sensitivity_y;
        camera_rotate(&cam_player, delta.x, delta.y);
    }

    camera_update(&cam_player);

    ////////////////////////////////////////////////////////////////////////
    // Render
    ////////////////////////////////////////////////////////////////////////
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

static int save_file()
{
    enum rico_error err;

    if (RICO_CHUNK_DEFAULT.uid.uid == UID_NULL)
        return RICO_ERROR(ERR_CHUNK_NULL, "Failed to save NULL chunk");

    struct rico_file file;
    err = rico_file_open_write(&file, "chunks/cereal.bin",
                               RICO_FILE_VERSION_CURRENT);
    if (err) return err;

    err = rico_serialize(&RICO_CHUNK_DEFAULT, &file);
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

    err = rico_serialize(&RICO_CHUNK_DEFAULT, &backupFile);
    rico_file_close(&backupFile);

    return err;
}

////////////////////////////////////////////////////////////////////////
// Camera forward ray v. scene
////////////////////////////////////////////////////////////////////////
static void select_first_obj()
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

static int handle_engine_events(SDL_Event event, bool *handled)
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
    else if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.mod & KMOD_CTRL &&
            event.key.keysym.mod & KMOD_SHIFT) {}
        else if (event.key.keysym.mod & KMOD_CTRL &&
                 event.key.keysym.mod & KMOD_ALT) {}
        else if (event.key.keysym.mod & KMOD_SHIFT)
        {
            // [`]: DEBUG: Show a test string for 3 seconds
            if (event.key.keysym.sym == SDLK_BACKQUOTE)
            {
                err = string_init("STR_EDIT_MODE", STR_SLOT_DYNAMIC, 0, 0,
                                  COLOR_GREEN, 3000, RICO_FONT_DEFAULT,
                                  "Blah blah testing some stuff.\n3333");
                if (err) return err;
                *handled = true;
            }
        }
        else if (event.key.keysym.mod & KMOD_CTRL)
        {
            // [Ctrl-S]: Save chunk
            if (event.key.keysym.sym == SDLK_s)
            {
                save_file();
                *handled = true;
            }
        }
        else if (event.key.keysym.mod & KMOD_ALT)
        {}
        // [2]: Toggle FPS counter
        else if (event.key.keysym.sym == SDLK_2)
        {
            fps_render = !fps_render;
            if (!fps_render)
                string_free(STR_SLOT_FPS);
            *handled = true;
        }
        // [Esc]: Save and exit
        else if (event.key.keysym.sym == SDLK_ESCAPE)
        {
            err = string_init("STR_CONFIRM_QUIT", STR_SLOT_MENU_QUIT, 600,
                              400, COLOR_GREEN, 0, RICO_FONT_DEFAULT,
                              "Are you sure you want to quit?\n" \
                              "                              \n" \
                              "      Yes (Y) or No (N)       ");
            if (err) return err;
            state = STATE_MENU_QUIT;
            *handled = true;
        }
        // [L]: Toggle scene lighting
        else if (event.key.keysym.sym == SDLK_l)
        {
            // TODO: Pretty sure this is broken
            // TODO: Use this to change shader program on render
            enable_lighting = !enable_lighting;
            *handled = true;
        }
        // [M]: Toggle mouse lock-to-window
        else if (event.key.keysym.sym == SDLK_m)
        {
            mouse_lock = !mouse_lock;
            SDL_SetRelativeMouseMode(mouse_lock);
            *handled = true;
        }
        // [P]: DEBUG: Force breakpoint
        else if (event.key.keysym.sym == SDLK_p)
        {
            // SDL_TriggerBreakpoint();
            // *handled = true;
        }
    }

    return err;
}

static int handle_camera_events(SDL_Event event, bool *handled)
{
    struct vec3 delta = VEC3_ZERO;

    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.mod & KMOD_CTRL &&
            event.key.keysym.mod & KMOD_SHIFT) {}
        else if (event.key.keysym.mod & KMOD_CTRL &&
                 event.key.keysym.mod & KMOD_ALT) {}
        //else if (event.key.keysym.mod & KMOD_SHIFT) {}
        else if (event.key.keysym.mod & KMOD_CTRL) {}
        else if (event.key.keysym.mod & KMOD_ALT) {}

        // [Shift]: Sprint
        else if (event.key.keysym.sym == SDLK_LSHIFT)
        {
            sprint = true;
            *handled = true;
        }
        // [1]: Toggle wireframe mode
        else if (event.key.keysym.sym == SDLK_1)
        {
            cam_player.fill_mode = (cam_player.fill_mode == GL_FILL)
                ? GL_LINE
                : GL_FILL;
            *handled = true;
        }
        // [R]: Toggle sprint (fast camera)
        else if (event.key.keysym.sym == SDLK_r)
        {
            snail = !snail;
            *handled = true;
        }
        // [Q]: Move camera down
        else if (event.key.keysym.sym == sdlk[RICOKEY_DOWN] &&
                 !event.key.repeat)
        {
            pressed[RICOKEY_DOWN] = true;
            delta.y -= 1.0f;
            *handled = true;
        }
        // [E]: Move camera up
        else if (event.key.keysym.sym == sdlk[RICOKEY_UP] &&
                 !event.key.repeat)
        {
            pressed[RICOKEY_UP] = true;
            delta.y += 1.0f;
            *handled = true;
        }
        // [A]: Move camera left
        else if (event.key.keysym.sym == sdlk[RICOKEY_LEFT] &&
                 !event.key.repeat)
        {
            pressed[RICOKEY_LEFT] = true;
            delta.x -= 1.0f;
            *handled = true;
        }
        // [D]: Move camera right
        else if (event.key.keysym.sym == sdlk[RICOKEY_RIGHT] &&
                 !event.key.repeat)
        {
            pressed[RICOKEY_RIGHT] = true;
            delta.x += 1.0f;
            *handled = true;
        }
        // [W]: Move camera forward
        else if (event.key.keysym.sym == sdlk[RICOKEY_FORWARD] &&
                 !event.key.repeat)
        {
            pressed[RICOKEY_FORWARD] = true;
            delta.z += 1.0f;
            *handled = true;
        }
        // [S]: Move camera backward
        else if (event.key.keysym.sym == sdlk[RICOKEY_BACKWARD] &&
                 !event.key.repeat)
        {
            pressed[RICOKEY_BACKWARD] = true;
            delta.z -= 1.0f;
            *handled = true;
        }
        // [F]: Reset camera to starting position
        else if (event.key.keysym.sym == SDLK_f)
        {
            //TODO: FIX ME
            // camera_reset(&camera);
            cam_player.position.y = 1.7f;
            cam_player.need_update = true;
            *handled = true;
        }
        // [C]: Toggle locked camera
        else if (event.key.keysym.sym == SDLK_c)
        {
            cam_player.locked = !cam_player.locked;
            *handled = true;
        }
    }
    else if (event.type == SDL_KEYUP)
    {
        // [Shift]: Sprint
        if (event.key.keysym.sym == SDLK_LSHIFT)
        {
            sprint = false;
            *handled = true;
        }
        else if (pressed[RICOKEY_DOWN] &&
                 event.key.keysym.sym == sdlk[RICOKEY_DOWN])
        {
            pressed[RICOKEY_DOWN] = false;
            delta.y += 1.0f;
            *handled = true;
        }
        else if (pressed[RICOKEY_UP] &&
                 event.key.keysym.sym == sdlk[RICOKEY_UP])
        {
            pressed[RICOKEY_UP] = false;
            delta.y -= 1.0f;
            *handled = true;
        }
        else if (pressed[RICOKEY_LEFT] &&
                 event.key.keysym.sym == sdlk[RICOKEY_LEFT])
        {
            pressed[RICOKEY_LEFT] = false;
            delta.x += 1.0f;
            *handled = true;
        }
        else if (pressed[RICOKEY_RIGHT] &&
                 event.key.keysym.sym == sdlk[RICOKEY_RIGHT])
        {
            pressed[RICOKEY_RIGHT] = false;
            delta.x -= 1.0f;
            *handled = true;
        }
        else if (pressed[RICOKEY_FORWARD] &&
                 event.key.keysym.sym == sdlk[RICOKEY_FORWARD])
        {
            pressed[RICOKEY_FORWARD] = false;
            delta.z -= 1.0f;
            *handled = true;
        }
        else if (pressed[RICOKEY_BACKWARD] &&
                 event.key.keysym.sym == sdlk[RICOKEY_BACKWARD])
        {
            pressed[RICOKEY_BACKWARD] = false;
            delta.z += 1.0f;
            *handled = true;
        }
    }

    if (*handled)
    {
        if (!vec3_equals(&delta, &VEC3_ZERO))
        {
            vec3_normalize(&delta);
            vec3_add(&view_acc, &delta);
        }
    }

    return SUCCESS;
}

static int handle_edit_events(SDL_Event event, bool *handled)
{
    RICO_ASSERT(state_is_edit());

    enum rico_error err;

    err = handle_engine_events(event, handled);
    if (err)     return err;
    if (*handled) return SUCCESS;

    err = handle_camera_events(event, handled);
    if (err)     return err;
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
    else if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.mod & KMOD_CTRL &&
            event.key.keysym.mod & KMOD_SHIFT) {}
        else if (event.key.keysym.mod & KMOD_CTRL &&
                 event.key.keysym.mod & KMOD_ALT) {}
        else if (event.key.keysym.mod & KMOD_SHIFT)
        {
            // [Shift-Tab]: Cycle select through objects (in reverse)
            if (event.key.keysym.sym == SDLK_TAB)
            {
                select_prev_obj();
                *handled = true;
            }
            // [Shift-B]: Recalculate bounding boxes of all objects
            else if (event.key.keysym.sym == SDLK_b)
            {
                recalculate_all_bbox();
                *handled = true;
            }
        }
        else if (event.key.keysym.mod & KMOD_CTRL)
        {
            // [Ctrl-D]: Duplicate selected object
            if (event.key.keysym.sym == SDLK_d)
            {
                selected_duplicate();
                *handled = true;
            }
        }
        else if (event.key.keysym.mod & KMOD_ALT) {}

        // [`]: Exit edit mode
        else if (event.key.keysym.sym == SDLK_BACKQUOTE)
        {
            string_free(STR_SLOT_SELECTED_OBJ);
            state = STATE_PLAY_EXPLORE;
            *handled = true;
        }
        // [Numpad 0]: Select next edit mode
        else if (event.key.keysym.sym == SDLK_KP_0)
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
        else if (event.key.keysym.sym == SDLK_KP_PERIOD)
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
        else if (event.key.keysym.sym == SDLK_TAB)
        {
            select_next_obj();
            *handled = true;
        }
        // [Ins]: Create new object
        else if (event.key.keysym.sym == SDLK_INSERT)
        {
            create_obj();
            *handled = true;
        }
        // [Del]: Delete selected object
        else if (event.key.keysym.sym == SDLK_DELETE)
        {
            selected_delete();
            *handled = true;
        }
    }

    return SUCCESS;
}

static int state_play_explore()
{
    enum rico_error err = SUCCESS;

    SDL_Event event;
    while (state == STATE_PLAY_EXPLORE && SDL_PollEvent(&event))
    {
        bool handled = 0;

        err = handle_engine_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        err = handle_camera_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}

            // [`]: Enter edit mode
            else if (event.key.keysym.sym == SDLK_BACKQUOTE) // backtick
            {
                state = STATE_EDIT_TRANSLATE;
                selected_print();
            }
        }
    }

    return err;
}

static int state_edit_translate()
{
    enum rico_error err = SUCCESS;

    struct vec3 translate = VEC3_ZERO;
    bool translate_reset = false;
    bool trans_delta_changed = false;

    SDL_Event event;
    while (state == STATE_EDIT_TRANSLATE && SDL_PollEvent(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}
            // [0]: Reset selected object's translation
            else if (event.key.keysym.sym == SDLK_0)
                translate_reset = true;
            // [Up Arrow]: Translate selected object up
            else if (event.key.keysym.sym == SDLK_UP)
                translate.y += trans_delta;
            // [Down Arrow]: Translate selected object down
            else if (event.key.keysym.sym == SDLK_DOWN)
                translate.y -= trans_delta;
            // [Left Arrow]: Translate selected object west
            else if (event.key.keysym.sym == SDLK_LEFT)
                translate.x -= trans_delta;
            // [Right Arrow]: Translate selected object east
            else if (event.key.keysym.sym == SDLK_RIGHT)
                translate.x += trans_delta;
            // [Page Up]: Translate selected object north
            else if (event.key.keysym.sym == SDLK_PAGEUP)
                translate.z -= trans_delta;
            // [Page Down]: Translate selected object south
            else if (event.key.keysym.sym == SDLK_PAGEDOWN)
                translate.z += trans_delta;
            // [+ / Numpad +]: Increase translation delta
            else if (event.key.keysym.sym == SDLK_PLUS ||
                     event.key.keysym.sym == SDLK_KP_PLUS)
            {
                if (trans_delta < TRANS_DELTA_MAX)
                {
                    trans_delta *= 10.0f;
                    if (trans_delta > TRANS_DELTA_MAX)
                        trans_delta = TRANS_DELTA_MAX;

                    trans_delta_changed = true;
                }
            }
            // [- / Numpad -]: Decrease translation delta
            else if (event.key.keysym.sym == SDLK_MINUS ||
                     event.key.keysym.sym == SDLK_KP_MINUS)
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
    else if (!vec3_equals(&translate, &VEC3_ZERO))
    {
        selected_translate(&cam_player, &translate);
    }

    if (trans_delta_changed)
    {
        char buf[50] = { 0 };
        sprintf(buf, "Trans Delta: %f", trans_delta);
        err = string_init("STR_EDIT_TRANS_DELTA", STR_SLOT_EDIT_INFO, 0, 0,
                          COLOR_DARK_BLUE_HIGHLIGHT, 1000, RICO_FONT_DEFAULT,
                          buf);
        if (err) return err;
    }

    return err;
}

static int state_edit_rotate()
{
    enum rico_error err = SUCCESS;

    struct vec3 rotate = VEC3_ZERO;
    bool rotate_reset = false;
    bool rot_delta_changed = false;

    SDL_Event event;
    while (state == STATE_EDIT_ROTATE && SDL_PollEvent(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}
            // [7]: Toggle heptagonal rotations
            else if (event.key.keysym.sym == SDLK_7)
            {
                if (rot_delta == ROT_DELTA_DEFAULT)
                    rot_delta = (float)M_SEVENTH_DEG;
                else
                    rot_delta = ROT_DELTA_DEFAULT;
            }
            // [0]: Reset selected object's rotation
            else if (event.key.keysym.sym == SDLK_0)
                rotate_reset = true;
            // [Up Arrow]: Rotate selected object up
            else if (event.key.keysym.sym == SDLK_UP)
                rotate.x -= rot_delta;
            // [Down Arrow]: Rotate selected object down
            else if (event.key.keysym.sym == SDLK_DOWN)
                rotate.x += rot_delta;
            // [Left Arrow]: Rotate selected object west
            else if (event.key.keysym.sym == SDLK_LEFT)
                rotate.y -= rot_delta;
            // [Right Arrow]: Rotate selected object east
            else if (event.key.keysym.sym == SDLK_RIGHT)
                rotate.y += rot_delta;
            // [Page Up]: Rotate selected object north
            else if (event.key.keysym.sym == SDLK_PAGEUP)
                rotate.z -= rot_delta;
            // [Page Down]: Rotate selected object south
            else if (event.key.keysym.sym == SDLK_PAGEDOWN)
                rotate.z += rot_delta;
            // [+ / Numpad +]: Increase rotation delta
            else if (event.key.keysym.sym == SDLK_PLUS ||
                     event.key.keysym.sym == SDLK_KP_PLUS)
            {
                if (rot_delta < ROT_DELTA_MAX)
                {
                    rot_delta += (rot_delta < 5.0f) ? 1.0f : 5.0f;
                    if (rot_delta > ROT_DELTA_MAX)
                        rot_delta = ROT_DELTA_MAX;

                    rot_delta_changed = true;
                }
            }
            // [- / Numpad -]: Decrease rotation delta
            else if (event.key.keysym.sym == SDLK_MINUS ||
                     event.key.keysym.sym == SDLK_KP_MINUS)
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
    else if (!vec3_equals(&rotate, &VEC3_ZERO))
    {
        selected_rotate(&rotate);
    }

    if (rot_delta_changed)
    {
        char buf[50] = { 0 };
        sprintf(buf, "Rot Delta: %f", rot_delta);
        err = string_init("STR_EDIT_ROT_DELTA", STR_SLOT_EDIT_INFO, 0, 0,
                          COLOR_DARK_BLUE_HIGHLIGHT, 1000, RICO_FONT_DEFAULT,
                          buf);
        if (err) return err;
    }

    return err;
}

static int state_edit_scale()
{
    enum rico_error err = SUCCESS;

    struct vec3 scale = VEC3_ZERO;
    bool scale_reset = false;
    bool scale_delta_changed = false;

    SDL_Event event;
    while (state == STATE_EDIT_SCALE && SDL_PollEvent(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}
            // [0]: Reset selected object's scale
            else if (event.key.keysym.sym == SDLK_0)
                scale_reset = true;
            // [Up Arrow]: Scale selected object up
            else if (event.key.keysym.sym == SDLK_UP)
                scale.y += scale_delta;
            // [Down Arrow]: Scale selected object down
            else if (event.key.keysym.sym == SDLK_DOWN)
                scale.y -= scale_delta;
            // [Left Arrow]: Scale selected object west
            else if (event.key.keysym.sym == SDLK_LEFT)
                scale.x -= scale_delta;
            // [Right Arrow]: Scale selected object east
            else if (event.key.keysym.sym == SDLK_RIGHT)
                scale.x += scale_delta;
            // [Page Up]: Scale selected object north
            else if (event.key.keysym.sym == SDLK_PAGEUP)
                scale.z += scale_delta;
            // [Page Down]: Scale selected object south
            else if (event.key.keysym.sym == SDLK_PAGEDOWN)
                scale.z -= scale_delta;
            // [+ / Numpad +]: Increase scale delta
            else if (event.key.keysym.sym == SDLK_PLUS ||
                     event.key.keysym.sym == SDLK_KP_PLUS)
            {
                if (scale_delta < SCALE_DELTA_MAX)
                {
                    scale_delta += (scale_delta < 1.0f) ? 0.1f : 1.0f;
                    if (scale_delta > SCALE_DELTA_MAX)
                        scale_delta = SCALE_DELTA_MAX;

                    scale_delta_changed = true;
                }
            }
            // [- / Numpad -]: Decrease scale delta
            else if (event.key.keysym.sym == SDLK_MINUS ||
                     event.key.keysym.sym == SDLK_KP_MINUS)
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
    else if (!vec3_equals(&scale, &VEC3_ZERO))
    {
        selected_scale(&scale);
    }

    if (scale_delta_changed)
    {
        char buf[50] = { 0 };
        sprintf(buf, "Scale Delta: %f", scale_delta);
        err = string_init("STR_EDIT_SCALE_DELTA", STR_SLOT_EDIT_INFO, 0, 0,
                          COLOR_DARK_BLUE_HIGHLIGHT, 1000, RICO_FONT_DEFAULT,
                          buf);
        if (err) return err;
    }

    return err;
}

static int state_edit_texture()
{
    enum rico_error err = SUCCESS;

    SDL_Event event;
    while (state == STATE_EDIT_TEXTURE && SDL_PollEvent(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN)
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

static int state_edit_mesh()
{
    enum rico_error err = SUCCESS;

    SDL_Event event;
    while (state == STATE_EDIT_MESH && SDL_PollEvent(&event))
    {
        bool handled = 0;

        err = handle_edit_events(event, &handled);
        if (err)     return err;
        if (handled) continue;

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.mod & KMOD_CTRL &&
                event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL &&
                     event.key.keysym.mod & KMOD_ALT) {}
            else if (event.key.keysym.mod & KMOD_SHIFT) {}
            else if (event.key.keysym.mod & KMOD_CTRL) {}
            else if (event.key.keysym.mod & KMOD_ALT) {}

            // [Left Arrow]: Cycle selected object's mesh (in reverse)
            else if (event.key.keysym.sym == SDLK_LEFT)
                selected_mesh_prev();
            // [Right Arrow]: Cycle selected object's mesh
            else if (event.key.keysym.sym == SDLK_RIGHT)
                selected_mesh_next();
            // [B]: Recalculate bounding box based on current mesh
            else if (event.key.keysym.sym == SDLK_b)
                selected_bbox_reset();
        }
    }

    return err;
}

static int state_menu_quit()
{
    enum rico_error err = SUCCESS;

    SDL_Event event;
    while (state == STATE_MENU_QUIT && SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
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

            // [Y]: Confirm: Save and exit
            else if (event.key.keysym.sym == SDLK_y)
            {
                string_free(STR_SLOT_MENU_QUIT);
                save_file();
                state = STATE_ENGINE_SHUTDOWN;
            }
            // [N] / [Escape]: Abort: Return to play mode
            else if (event.key.keysym.sym == SDLK_n ||
                     event.key.keysym.sym == SDLK_ESCAPE)
            {
                string_free(STR_SLOT_MENU_QUIT);
                state = STATE_PLAY_EXPLORE;
                selected_print();
            }
        }
    }

    return err;
}

static int state_text_input()
{
    return SUCCESS;
}

static void rico_init_cereal()
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

static int rico_init_pools()
{
    enum rico_error err;

    // TODO: Don't initialize these manually this way if they're going to be
    //       deserialized from chunks. Instead, create a hard-coded test chunk
    //       that gets loaded if the real chunk can't be loaded from the save
    //       file.

    err = chunk_init("RICO_CHUNK_DEFAULT", 0,
                     RICO_STRING_POOL_SIZE,
                     RICO_FONT_POOL_SIZE,
                     RICO_TEXTURE_POOL_SIZE,
                     RICO_MATERIAL_POOL_SIZE,
                     RICO_MESH_POOL_SIZE,
                     RICO_OBJECT_POOL_SIZE,
                     &RICO_CHUNK_DEFAULT);
    if (err) return err;

    // TODO: Remove the self-hosted pool points from all of the classes below
    //       and send them chunk pointers instead.

    /*
    err = rico_string_init(RICO_STRING_POOL_SIZE);
    if (err) return err;

    err = rico_font_init(RICO_FONT_POOL_SIZE);
    if (err) return err;

    err = rico_texture_init(RICO_TEXTURE_POOL_SIZE);
    if (err) return err;

    err = rico_material_init(RICO_MATERIAL_POOL_SIZE);
    if (err) return err;

    err = rico_mesh_init(RICO_MESH_POOL_SIZE);
    if (err) return err;

    err = rico_object_init(RICO_OBJECT_POOL_SIZE);
    if (err) return err;
    */

    return err;
}

static int rico_init_fonts()
{
    enum rico_error err;

    // TODO: Use static slots to allocate default resources
    err = font_init("font/courier_new.bff", &RICO_FONT_DEFAULT);
    return err;
}

static int rico_init_textures()
{
    enum rico_error err;

    // TODO: Use static slots to allocate default resources
    err = texture_load_file("TEXTURE_DEFAULT_DIFF", GL_TEXTURE_2D,
                            "texture/basic_diff.tga", 32,
                            &RICO_TEXTURE_DEFAULT_DIFF);
    if (err) return err;

    err = texture_load_file("TEXTURE_DEFAULT_SPEC", GL_TEXTURE_2D,
                            "texture/basic_spec.tga", 32,
                            &RICO_TEXTURE_DEFAULT_SPEC);
    return err;
}

static int rico_init_materials()
{
    enum rico_error err;

    // TODO: Use static slots to allocate default resources
    err = material_init("MATERIAL_DEFAULT", RICO_TEXTURE_DEFAULT_DIFF,
                        RICO_TEXTURE_DEFAULT_SPEC, 0.5f,
                        &RICO_MATERIAL_DEFAULT);
    return err;
}

static int rico_init_meshes()
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

    err = mesh_load(&RICO_MESH_DEFAULT, "MESH_DEFAULT", MESH_OBJ_WORLD,
                    vert_count, vertices, element_count, elements,
                    GL_STATIC_DRAW);
    if (err) return err;

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
    printf("[perf][mesh] Meshes loaded in: %d ticks\n", ticks2 - ticks);

    return err;
}

static int rico_init()
{
    enum rico_error err;

    rico_init_cereal();

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing pools\n");
    printf("------------------------------------------------------------\n");
    err = rico_init_pools();
    if (err) return err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing fonts\n");
    printf("------------------------------------------------------------\n");
    err = rico_init_fonts();
    if (err) return err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing textures\n");
    printf("------------------------------------------------------------\n");
    err = rico_init_textures();
    if (err) return err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing materials\n");
    printf("------------------------------------------------------------\n");
    err = rico_init_materials();
    if (err) return err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing meshes\n");
    printf("------------------------------------------------------------\n");
    err = rico_init_meshes();
    if (err) return err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing primitives\n");
    printf("------------------------------------------------------------\n");
    prim_init(PRIM_SEGMENT);
    prim_init(PRIM_RAY);

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing game world\n");
    printf("------------------------------------------------------------\n");
    err = init_glref();
    if (err) return err;

    if (reset_game_world)
    {
        printf("Loading hard-coded test chunk\n");
        err = init_hardcoded_test_chunk();
        if (err) return err;
    }
    else
    {
        struct rico_file file;
        err = rico_file_open_read(&file, "chunks/cereal.bin");
        if (err) return err;

        err = rico_deserialize(&RICO_CHUNK_DEFAULT, &file);
        rico_file_close(&file);
        if (err) return err;
    }

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing camera\n");
    printf("------------------------------------------------------------\n");
    camera_reset(&cam_player);
    return err;
}

static int state_engine_init()
{
    enum rico_error err;

    printf("------------------------------------------------------------\n");
    printf("#         ______            _______        _               #\n");
    printf("#         |  __ \\ O        |__   __|      | |              #\n");
    printf("#         | |__| |_  ___ ___  | | ___  ___| |__            #\n");
    printf("#         |  _  /| |/ __/ _ \\ | |/ _ \\/ __| '_ \\           #\n");
    printf("#         | | \\ \\| | |_| (_) || |  __/ |__| | | |          #\n");
    printf("#         |_|  \\_\\_|\\___\\___/ |_|\\___|\\___|_| |_|          #\n");
    printf("#                                                          #\n");
    printf("#               Copyright 2017 Dan Bechard                 #\n");
    printf("------------------------------------------------------------\n");

    // TODO: Where does this belong? Needs access to "mouse_lock".
    SDL_SetRelativeMouseMode(mouse_lock);

    perfs_frequency = SDL_GetPerformanceFrequency();
    last_perfs = SDL_GetPerformanceCounter();
    last_cycles = __rdtsc();
    fps_last_render = last_perfs;
    view_vel = VEC3_ZERO;
    view_acc = VEC3_ZERO;

    // Initialize key map
    sdlk[RICOKEY_RIGHT]     = SDLK_d;
    sdlk[RICOKEY_LEFT]      = SDLK_a;
    sdlk[RICOKEY_UP]        = SDLK_e;
    sdlk[RICOKEY_DOWN]      = SDLK_q;
    sdlk[RICOKEY_FORWARD]   = SDLK_w;
    sdlk[RICOKEY_BACKWARD]  = SDLK_s;

    err = rico_init();
    if (err) return err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][  GO] Initialization complete. Starting game.\n");
    printf("------------------------------------------------------------\n");

    state = STATE_PLAY_EXPLORE;
    return err;
}

static int state_engine_shutdown()
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