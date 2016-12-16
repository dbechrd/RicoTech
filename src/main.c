#ifdef __APPLE__
#define __gl_h_
//#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

#include "const.h"
#include "camera.h"
#include "util.h"
#include "glref.h"
#include "rico_texture.h"
#include "rico_mesh.h"
#include "rico_object.h"
#include "load_object.h"
#include "rico_chunk.h"
#include "rico_cereal.h"
#include "rico_font.h"
#include "primitives.h"
#include "test_geom.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_TGA
#include "stb/stb_image.h"
#include "GL/gl3w.h"
#include "SDL/SDL.h"

static SDL_Window *window = NULL;
static SDL_GLContext context = NULL;
static struct camera camera;
static const bool reset_game_world = false;
static struct rico_chunk first_chunk;
static enum rico_edit_mode edit_mode = EDIT_TRANSLATE;

// This is really stupid, move it somewhere else
static u32 meshes[RICO_MESH_POOL_SIZE];
static u32 mesh_count;

static inline void init_stb()
{
    stbi_set_flip_vertically_on_load(1);
}

static int sdl_gl_attrib(SDL_GLattr attr, int value)
{
    int sdl_err = SDL_GL_SetAttribute(attr, value);
    if (sdl_err < 0)
    {
        fprintf(stderr, "SDL_GL_SetAttribute %d error: %s\n", attr,
                SDL_GetError());
        return RICO_ERROR(ERR_SDL_INIT);
    }
    return SUCCESS;
}

static int init_sdl()
{
    enum rico_error err;
    int sdl_err;

    sdl_err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (sdl_err < 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return RICO_ERROR(ERR_SDL_INIT);
    }

/*
SDL_GL_RED_SIZE  (defaults to 3)
the minimum number of bits for the red channel of the color buffer;

SDL_GL_GREEN_SIZE  (defaults to 3)
the minimum number of bits for the green channel of the color buffer;

SDL_GL_BLUE_SIZE  (defaults to 2)
the minimum number of bits for the blue channel of the color buffer;

SDL_GL_ALPHA_SIZE  (defaults to 0)
the minimum number of bits for the alpha channel of the color buffer;

SDL_GL_BUFFER_SIZE  (defaults to 0)
the minimum number of bits for frame buffer size;

SDL_GL_DOUBLEBUFFER  (defaults to on)
whether the output is single or double buffered;

SDL_GL_DEPTH_SIZE  (defaults to 16)
the minimum number of bits in the depth buffer;

SDL_GL_STENCIL_SIZE  (defaults to 0)
the minimum number of bits in the stencil buffer;

SDL_GL_ACCUM_RED_SIZE  (defaults to 0)
the minimum number of bits for the red channel of the accumulation buffer;

SDL_GL_ACCUM_GREEN_SIZE  (defaults to 0)
the minimum number of bits for the green channel of the accumulation buffer;

SDL_GL_ACCUM_BLUE_SIZE  (defaults to 0)
the minimum number of bits for the blue channel of the accumulation buffer;

SDL_GL_ACCUM_ALPHA_SIZE  (defaults to 0)
the minimum number of bits for the alpha channel of the accumulation buffer;

SDL_GL_STEREO  (defaults to off)
whether the output is stereo 3D;

SDL_GL_MULTISAMPLEBUFFERS  (defaults to 0)
the number of buffers used for multisample anti-aliasing;

SDL_GL_MULTISAMPLESAMPLES  (defaults to 0)
the number of samples used around the pixel used for multisample anti-aliasing;

SDL_GL_ACCELERATED_VISUAL  (defaults to allow either)
set to 1 to require hardware acceleration, set to 0 to force software rendering;

SDL_GL_CONTEXT_MAJOR_VERSION
OpenGL context major version

SDL_GL_CONTEXT_MINOR_VERSION
OpenGL context minor version

SDL_GL_CONTEXT_FLAGS  (defaults to 0)
some combination of 0 or more of elements of the SDL_GLcontextFlag enumeration;

SDL_GL_CONTEXT_PROFILE_MASK default is platform specific
type of GL context (Core, Compatibility, ES). See SDL_GLprofile;

SDL_GL_SHARE_WITH_CURRENT_CONTEXT  (defaults to 0)
OpenGL context sharing;

SDL_GL_FRAMEBUFFER_SRGB_CAPABLE  (defaults to 0 (>= SDL 2.0.1))
requests sRGB capable visual;

SDL_GL_CONTEXT_RELEASE_BEHAVIOR  (defaults to 1 (>= SDL 2.0.4))
sets context the release behavior;
*/

#if _DEBUG
    err = sdl_gl_attrib(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    if (err) return err;
#endif

    err = sdl_gl_attrib(SDL_GL_RED_SIZE,   8);          if (err) return err;
    err = sdl_gl_attrib(SDL_GL_GREEN_SIZE, 8);          if (err) return err;
    err = sdl_gl_attrib(SDL_GL_BLUE_SIZE,  8);          if (err) return err;
    err = sdl_gl_attrib(SDL_GL_ALPHA_SIZE, 8);          if (err) return err;

    err = sdl_gl_attrib(SDL_GL_DOUBLEBUFFER, 1);        if (err) return err;
    err = sdl_gl_attrib(SDL_GL_DEPTH_SIZE, 24);         if (err) return err;

    err = sdl_gl_attrib(SDL_GL_MULTISAMPLEBUFFERS, 1);  if (err) return err;
    err = sdl_gl_attrib(SDL_GL_MULTISAMPLESAMPLES, 16); if (err) return err;
    //err = sdl_gl_attrib(SDL_GL_ACCELERATED_VISUAL, 1);  if (err) return err;

    err = sdl_gl_attrib(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    //                  SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    if (err) return err;

    err = sdl_gl_attrib(SDL_GL_CONTEXT_MAJOR_VERSION, 3); if (err) return err;
    err = sdl_gl_attrib(SDL_GL_CONTEXT_MINOR_VERSION, 2); if (err) return err;

    // Create window
    window = SDL_CreateWindow("RicoTech", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H,
                              SDL_WINDOW_OPENGL);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        return RICO_ERROR(ERR_SDL_INIT);
    }

    // Create GL context
    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        fprintf(stderr, "SDL_GL_CreateContext error: %s\n", SDL_GetError());
        return RICO_ERROR(ERR_SDL_INIT);
    }

    // Disable V-sync
    sdl_err = SDL_GL_SetSwapInterval(0); //Default on
    if (sdl_err < 0) {
        fprintf(stderr, "SDL_GL_SetSwapInterval error: %s\n", SDL_GetError());
        return RICO_ERROR(ERR_SDL_INIT);
    }
    else
    {
        int swap = SDL_GL_GetSwapInterval();
        fprintf(stdout, "VSync = %s\n", (swap) ? "Enabled" : "Disabled");
    }

    return SUCCESS;
}

static int init_gl3w()
{
    if (gl3wInit())
    {
        fprintf(stderr, "gl3wInit failed.\n");
        getchar();
        return 1;
    }

    fprintf(stdout, "OpenGL = \"%s\"\nGLSL = \"%s\"\n", glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));

    if (!gl3wIsSupported(3, 2))
    {
        fprintf(stderr, "OpenGL 3.2 not supported.\n");
        getchar();
        return 1;
    }

    return 0;
}

static void init_opengl()
{
#if _DEBUG
    {
        GLint gl_max_vertex_attribs;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_max_vertex_attribs);
        printf("GL_MAX_VERTEX_ATTRIBS = %d\n", gl_max_vertex_attribs);
    }

    if (glDebugMessageCallback != NULL)
    {
        fprintf(stdout, "Registered glDebugMessageCallback.\n");
        fprintf(stderr, "[TYPE][SEVERITY][ID] MESSAGE\n");

        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglCallbackFunction, NULL);

        GLuint unusedIds = 0;
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                              &unusedIds, true);
    }
    else
    {
        fprintf(stderr, "glDebugMessageCallback not available.\n");
    }
#endif

    // Depth buffer
    glDepthFunc(GL_LEQUAL);  // Default GL_LESS.
    glEnable(GL_DEPTH_TEST); // Default off.

    // Multi-sampling
    glEnable(GL_MULTISAMPLE);

    // Alpha-blending
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

static void rico_init_cereal()
{
    // Custom serialiers
    RicoCereal[RICO_UID_CHUNK].save[0] = &chunk_serialize_0;
    RicoCereal[RICO_UID_CHUNK].load[0] = &chunk_deserialize_0;

    RicoCereal[RICO_UID_POOL].save[0] = &pool_serialize_0;
    RicoCereal[RICO_UID_POOL].load[0] = &pool_deserialize_0;

    RicoCereal[RICO_UID_OBJECT].save[0] = &object_serialize_0;
    RicoCereal[RICO_UID_OBJECT].load[0] = &object_deserialize_0;

    RicoCereal[RICO_UID_BBOX].save[0] = &bbox_serialize_0;
    RicoCereal[RICO_UID_BBOX].load[0] = &bbox_deserialize_0;
}

static int rico_init_pools()
{
    enum rico_error err;

    err = rico_font_init(RICO_TEXTURE_POOL_SIZE);
    if (err) return err;

    err = rico_texture_init(RICO_TEXTURE_POOL_SIZE);
    if (err) return err;

    err = rico_mesh_init(RICO_MESH_POOL_SIZE);
    return err;
}

static int rico_init_fonts()
{
    enum rico_error err;
    printf("Loading fonts\n");

    err = font_init("font/courier_new.bff", &RICO_FONT_DEFAULT);
    return err;
}

static int rico_init_textures()
{
    enum rico_error err;
    printf("Loading textures\n");

    err = texture_load_file("TEXTURE_DEFAULT", GL_TEXTURE_2D,
                            "texture/basic.tga", &RICO_TEXTURE_DEFAULT);
    return err;
}

static int rico_init_meshes()
{
    enum rico_error err;
    printf("Loading meshes\n");

    //--------------------------------------------------------------------------
    // Create default mesh (white rect)
    //--------------------------------------------------------------------------
    #define VERT_COUNT 4
    const struct mesh_vertex vertices[VERT_COUNT] = {
        {
            { -1.0f, -1.0f, 0.0f },     //Position
            { 1.0f, 1.0f, 1.0f, 1.0f }, //Color
            { 0.0f, 0.0f }              //UV-coords
        },
        {
            { 1.0f, -1.0f, 0.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        {
            { 1.0f, 1.0f, 0.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f }
        },
        {
            { -1.0f, 1.0f, 0.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f }
        }
    };
    #define ELEMENT_COUNT 6
    const GLuint elements[ELEMENT_COUNT] = { 0, 1, 3, 1, 2, 3 };

    err = mesh_load("MESH_DEFAULT", VERT_COUNT, vertices, ELEMENT_COUNT, elements,
                    GL_STATIC_DRAW, &RICO_MESH_DEFAULT);
    if (err) return err;

    u32 ticks = SDL_GetTicks();

    err = load_obj_file("model/conference.ric", meshes, &mesh_count);
    if (err) return err;

    u32 ticks2 = SDL_GetTicks();
    printf("Model loaded in: %d\n", ticks2 - ticks);

    err = load_obj_file("model/spawn.ric", meshes, &mesh_count);
    if (err) return err;

    return err;
}

static int rico_init()
{
    enum rico_error err;

    init_stb();
    init_sdl();
    init_gl3w();
    init_opengl();

    rico_init_cereal();

    err = rico_init_pools();
    if (err) return err;

    err = rico_init_fonts();
    if (err) return err;

    err = rico_init_textures();
    if (err) return err;

    err = rico_init_meshes();
    if (err) return err;

    prim_init(PRIM_LINE);

    err = init_glref(meshes, mesh_count);
    if (err) return err;

    if (reset_game_world)
    {
        printf("Loading hard-coded test chunk\n");
        err = init_hardcoded_test_chunk(meshes, mesh_count);
        if (err) return err;
    }
    else
    {
        struct rico_file file;
        err = rico_file_open_read(&file, "chunks/cereal.bin");
        if (err) return err;

        err = rico_deserialize(&first_chunk, &file);
        rico_file_close(&file);
        if (err) return err;

        object_pool_set_unsafe(&first_chunk.objects);
    }

    camera_reset(&camera);
    return err;
}

static int save_file()
{
    enum rico_error err;

    struct rico_file file;
    err = rico_file_open_write(&file, "../res/chunks/cereal.bin",
                               RICO_FILE_VERSION_CURRENT);
    if (err) return err;

    err = rico_serialize(&first_chunk, &file);
    rico_file_close(&file);
    return err;
}

void set_edit_mode(enum rico_edit_mode mode)
{
    edit_mode = mode;
    char buf[50] = {0};
    sprintf(buf, "Edit mode: %s",
            rico_edit_mode_string[edit_mode]);
    glref_debuginfo(buf, COLOR_DARK_RED_HIGHLIGHT);
}

int mymain()
{
    enum rico_error err = rico_init();
    if (err) return err;

    //test_geom();

    //Human walk speed empirically found to be 33 steps in 20 seconds. That
    //is approximately 1.65 steps per second. At 60 fps, that is 0.0275 steps
    //per frame. Typical walking stride is ~0.762 meters (30 inches). Distance
    //travelled per frame (60hz) is 0.762 * 0.0275 = 0.020955 ~= 0.021

    //TODO: This assumes 1/60th second (60fps), this should be 1.65 * dt.
    //GLfloat view_trans_delta = 0.001f;
    //GLfloat view_trans_delta = 1.65f;
    GLfloat view_trans_delta = 5.0f;

    struct vec3 view_trans_vel = VEC3_ZERO;

    const float TRANS_DELTA_MIN = 0.01f;
    const float TRANS_DELTA_MAX = 10.0f;
    const float TRANS_DELTA_DEFAULT = 1.0f;
    const float ROT_DELTA_MIN = 1.0f;
    const float ROT_DELTA_MAX = 90.0f;
    const float ROT_DELTA_DEFAULT = 5.0f;
    const float SCALE_DELTA_MIN = 0.1f;
    const float SCALE_DELTA_MAX = 5.0f;
    const float SCALE_DELTA_DEFAULT = 1.0f;
    float trans_delta = TRANS_DELTA_DEFAULT;
    float rot_delta = ROT_DELTA_DEFAULT;
    float scale_delta = SCALE_DELTA_DEFAULT;

    int mouse_dx, mouse_dy;

    bool d_down = false;
    bool s_down = false;
    bool sprint = true;
    bool ambient_light = true;
    bool mouse_lock = true;
    bool quit = false;

    SDL_SetRelativeMouseMode(mouse_lock);

    GLuint time = SDL_GetTicks();
    SDL_Event windowEvent;

    while (!quit)
    {
        mouse_dx = mouse_dy = 0;

        while (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
            else if (windowEvent.type == SDL_MOUSEMOTION)
            {
                mouse_dx = windowEvent.motion.xrel;
                mouse_dy = windowEvent.motion.yrel;
            }
            else if (windowEvent.type == SDL_KEYDOWN)
            {
                struct vec3 delta = { 0 };

                if (windowEvent.key.keysym.sym == SDLK_KP_0)
                {
                    edit_mode++;
                    if (edit_mode == EDIT_COUNT)
                        edit_mode = 0;
                    set_edit_mode(edit_mode);
                }
                else if (windowEvent.key.keysym.sym == SDLK_TAB)
                {
                    if (windowEvent.key.keysym.mod & KMOD_SHIFT)
                    {
                        select_prev_obj();
                    }
                    else
                    {
                        select_next_obj();
                    }
                }
                else if (windowEvent.key.keysym.sym == SDLK_0)
                {
                    switch (edit_mode) {
                    case EDIT_TRANSLATE:
                        selected_translate(&camera, &VEC3_ZERO);
                        break;
                    case EDIT_ROTATE:
                        selected_rotate(&VEC3_ZERO);
                        break;
                    case EDIT_SCALE:
                        selected_scale(&VEC3_ZERO);
                        break;
                    default: break;
                    }
                }
                else if (windowEvent.key.keysym.sym == SDLK_UP)
                {
                    switch (edit_mode) {
                    case EDIT_TRANSLATE:
                        delta.y = trans_delta;
                        selected_translate(&camera, &delta);
                        break;
                    case EDIT_ROTATE:
                        delta.x = -rot_delta;
                        selected_rotate(&delta);
                        break;
                    case EDIT_SCALE:
                        delta.y = scale_delta;
                        selected_scale(&delta);
                        break;
                    default: break;
                    }
                }
                else if (windowEvent.key.keysym.sym == SDLK_DOWN)
                {
                    switch (edit_mode) {
                    case EDIT_TRANSLATE:
                        delta.y = -trans_delta;
                        selected_translate(&camera, &delta);
                        break;
                    case EDIT_ROTATE:
                        delta.x = rot_delta;
                        selected_rotate(&delta);
                        break;
                    case EDIT_SCALE:
                        delta.y = -scale_delta;
                        selected_scale(&delta);
                        break;
                    default: break;
                    }
                }
                else if (windowEvent.key.keysym.sym == SDLK_LEFT)
                {
                    switch (edit_mode) {
                    case EDIT_TRANSLATE:
                        delta.x = -trans_delta;
                        selected_translate(&camera, &delta);
                        break;
                    case EDIT_ROTATE:
                        delta.y = -rot_delta;
                        selected_rotate(&delta);
                        break;
                    case EDIT_SCALE:
                        delta.x = -scale_delta;
                        selected_scale(&delta);
                        break;
                    default: break;
                    }
                }
                else if (windowEvent.key.keysym.sym == SDLK_RIGHT)
                {
                    switch (edit_mode) {
                    case EDIT_TRANSLATE:
                        delta.x = trans_delta;
                        selected_translate(&camera, &delta);
                        break;
                    case EDIT_ROTATE:
                        delta.y = rot_delta;
                        selected_rotate(&delta);
                        break;
                    case EDIT_SCALE:
                        delta.x = scale_delta;
                        selected_scale(&delta);
                        break;
                    default: break;
                    }
                }
                else if (windowEvent.key.keysym.sym == SDLK_PAGEUP)
                {
                    switch (edit_mode) {
                    case EDIT_TRANSLATE:
                        delta.z = -trans_delta;
                        selected_translate(&camera, &delta);
                        break;
                    case EDIT_ROTATE:
                        delta.z = -rot_delta;
                        selected_rotate(&delta);
                        break;
                    case EDIT_SCALE:
                        delta.z = scale_delta;
                        selected_scale(&delta);
                        break;
                    default: break;
                    }
                }
                else if (windowEvent.key.keysym.sym == SDLK_PAGEDOWN)
                {
                    switch (edit_mode) {
                    case EDIT_TRANSLATE:
                        delta.z = trans_delta;
                        selected_translate(&camera, &delta);
                        break;
                    case EDIT_ROTATE:
                        delta.z = rot_delta;
                        selected_rotate(&delta);
                        break;
                    case EDIT_SCALE:
                        delta.z = -scale_delta;
                        selected_scale(&delta);
                        break;
                    default: break;
                    }
                }
                else if (windowEvent.key.keysym.sym == SDLK_PLUS ||
                         windowEvent.key.keysym.sym == SDLK_KP_PLUS)
                {
                    switch (edit_mode) {
                    case EDIT_TRANSLATE:
                        if (trans_delta < TRANS_DELTA_MAX)
                        {
                            trans_delta *= 10.0f;
                            if (trans_delta > TRANS_DELTA_MAX)
                                trans_delta = TRANS_DELTA_MAX;

                            char buf[50] = {0};
                            sprintf(buf, "Trans Delta: %f",
                                    trans_delta);
                            glref_debuginfo(buf, COLOR_DARK_BLUE_HIGHLIGHT);
                        }
                        break;
                    case EDIT_ROTATE:
                        if (rot_delta < ROT_DELTA_MAX)
                        {
                            rot_delta += (rot_delta < 5.0f) ? 1.0f : 5.0f;
                            if (rot_delta > ROT_DELTA_MAX)
                                rot_delta = ROT_DELTA_MAX;

                            char buf[50] = {0};
                            sprintf(buf, "Rot Delta: %f",
                                    rot_delta);
                            glref_debuginfo(buf, COLOR_DARK_BLUE_HIGHLIGHT);
                        }
                        break;
                    case EDIT_SCALE:
                        if (scale_delta < SCALE_DELTA_MAX)
                        {
                            scale_delta += (scale_delta < 1.0f) ? 0.1f : 1.0f;
                            if (scale_delta > SCALE_DELTA_MAX)
                                scale_delta = SCALE_DELTA_MAX;

                            char buf[50] = {0};
                            sprintf(buf, "Scale Delta: %f",
                                    scale_delta);
                            glref_debuginfo(buf, COLOR_DARK_BLUE_HIGHLIGHT);
                        }
                        break;
                    default: break;
                    }
                }
                else if (windowEvent.key.keysym.sym == SDLK_MINUS ||
                         windowEvent.key.keysym.sym == SDLK_KP_MINUS)
                {
                    switch (edit_mode) {
                    case EDIT_TRANSLATE:
                        if (trans_delta > TRANS_DELTA_MIN)
                        {
                            trans_delta /= 10.0f;
                            if (trans_delta < TRANS_DELTA_MIN)
                                trans_delta = TRANS_DELTA_MIN;

                            char buf[50] = {0};
                            sprintf(buf, "Trans Delta: %f",
                                    trans_delta);
                            glref_debuginfo(buf, COLOR_DARK_BLUE_HIGHLIGHT);
                        }
                        break;
                    case EDIT_ROTATE:
                        if (rot_delta > ROT_DELTA_MIN)
                        {
                            rot_delta -= (rot_delta > 5.0f) ? 5.0f : 1.0f;
                            if (rot_delta < ROT_DELTA_MIN)
                                rot_delta = ROT_DELTA_MIN;

                            char buf[50] = {0};
                            sprintf(buf, "Rot Delta: %f",
                                    rot_delta);
                            glref_debuginfo(buf, COLOR_DARK_BLUE_HIGHLIGHT);
                        }
                        break;
                    case EDIT_SCALE:
                        if (scale_delta > SCALE_DELTA_MIN)
                        {
                            scale_delta -= (scale_delta > 1.0f) ? 1.0f : 0.1f;
                            if (scale_delta < SCALE_DELTA_MIN)
                                scale_delta = SCALE_DELTA_MIN;

                            char buf[50] = {0};
                            sprintf(buf, "Scale Delta: %f",
                                    scale_delta);
                            glref_debuginfo(buf, COLOR_DARK_BLUE_HIGHLIGHT);
                        }
                        break;
                    default: break;
                    }
                }
                else if (!windowEvent.key.repeat)
                {
                    if (windowEvent.key.keysym.sym == SDLK_r)
                    {
                        sprint = !sprint;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_q)
                    {
                        view_trans_vel.y -= view_trans_delta;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_e)
                    {
                        view_trans_vel.y += view_trans_delta;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_a)
                    {
                        view_trans_vel.x -= view_trans_delta;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_d
                          && windowEvent.key.keysym.mod & KMOD_CTRL)
                    {
                        selected_duplicate();
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_d)
                    {
                        d_down = true;
                        view_trans_vel.x += view_trans_delta;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_w)
                    {
                        view_trans_vel.z += view_trans_delta;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_s
                          && windowEvent.key.keysym.mod & KMOD_CTRL)
                    {
                        save_file();
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_s)
                    {
                        s_down = true;
                        view_trans_vel.z -= view_trans_delta;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_f)
                    {
                        //TODO: FIX ME
                        // camera_reset(&camera);
                        camera.position.y = 1.7f;
                        camera.need_update = true;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_c)
                    {
                        camera.locked = !camera.locked;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_l)
                    {
                        ambient_light = !ambient_light;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_p)
                    {
                        SDL_TriggerBreakpoint();
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_m)
                    {
                        mouse_lock = !mouse_lock;
                        SDL_SetRelativeMouseMode(mouse_lock);
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_1)
                    {
                        camera.fill_mode = (camera.fill_mode == GL_FILL)
                            ? GL_LINE
                            : GL_FILL;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_7)
                    {
                        if (rot_delta == ROT_DELTA_DEFAULT)
                            rot_delta = (float)M_SEVENTH_DEG;
                        else
                            rot_delta = ROT_DELTA_DEFAULT;
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_DELETE)
                    {
                        selected_delete();
                    }
                    else if (windowEvent.key.keysym.sym == SDLK_BACKQUOTE)
                    {
                        glref_debuginfo("Blah blah testing some stuff.\n3333",
                                        COLOR_GREEN);
                    }
                    else if (windowEvent.key.keysym.sym != SDLK_LCTRL &&
                             windowEvent.key.keysym.sym != SDLK_RCTRL &&
                             windowEvent.key.keysym.sym != SDLK_LALT &&
                             windowEvent.key.keysym.sym != SDLK_RALT &&
                             windowEvent.key.keysym.sym != SDLK_LSHIFT &&
                             windowEvent.key.keysym.sym != SDLK_RSHIFT)
                    {
                        printf("Unhandled key: %x\n", windowEvent.key.keysym.sym);
                    }
                }
            }
            else if (windowEvent.type == SDL_KEYUP)
            {
                if (windowEvent.key.keysym.sym == SDLK_q)
                {
                    view_trans_vel.y += view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_e)
                {
                    view_trans_vel.y -= view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_a)
                {
                    view_trans_vel.x += view_trans_delta;
                }
                else if (d_down && windowEvent.key.keysym.sym == SDLK_d)
                {
                    d_down = false;
                    view_trans_vel.x -= view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_w)
                {
                    view_trans_vel.z -= view_trans_delta;
                }
                else if (s_down && windowEvent.key.keysym.sym == SDLK_s)
                {
                    s_down = false;
                    view_trans_vel.z += view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = true;
                    break;
                }
            }

            //TODO: Handle resize event
            /*if (windowEvent.type == SDL_WINDOWEVENT_SIZE)
            {
                glViewport(0, 0, windowEvent.window.event.size.width,
                           event.size.height);
                break;
            }*/
        }
        if (quit) break;

        ////////////////////////////////////////////////////////////////////////
        // Update time
        ////////////////////////////////////////////////////////////////////////
        GLuint newTime = SDL_GetTicks();
        GLfloat dt = (float)(newTime - time) / 1000.0f;
        time = newTime;

        ////////////////////////////////////////////////////////////////////////
        // Update camera
        ////////////////////////////////////////////////////////////////////////
        if (!vec3_equals(&view_trans_vel, &VEC3_ZERO))
        {
            struct vec3 view_delta = view_trans_vel;
            vec3_scale(&view_delta, dt);

            if (sprint) {
                //Debug: Sprint vertically.. yeah.. what?
                vec3_scale(&view_delta, 5.0f);

                //TODO: Only allow sprinting on the ground during normal play
                //view_delta.x *= 5.0f;
                //view_delta.z *= 5.0f;
            }

            camera_translate(&camera, &view_delta);
        }

        if (mouse_dx != 0 || mouse_dy != 0)
        {
            camera_rotate(&camera, mouse_dx, mouse_dy);
        }

        camera_update(&camera);

        ////////////////////////////////////////////////////////////////////////
        // Render
        ////////////////////////////////////////////////////////////////////////
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        //glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, camera.fill_mode);

        glref_update(dt, ambient_light);
        glref_render(&camera);
        camera_render(&camera);

        SDL_GL_SwapWindow(window);
    }

    //====================================================================
    // Cleanup
    //====================================================================
    free_glref();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    enum rico_error err = RICO_ERROR(mymain());
    if (err) getchar();

    return err;
}

///////////////////////////////////////////////////////////////////

//TODO: Make buffer somewhere else.. idk where yet
// static GLuint orig_make_buffer(const GLenum target, const void *buffer_data,
//                                const GLsizei buffer_size)
// {
//     GLuint buffer = 0;
//     glGenBuffers(1, &buffer);
//     glBindBuffer(target, buffer);
//     glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
//     return buffer;
// }
