#ifdef __APPLE__
#define __gl_h_
//#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

#include "const.h"
#include "camera.h"
#include "util.h"
#include "glref.h"
#include "rico_texture.h"
#include "rico_material.h"
#include "rico_mesh.h"
#include "rico_object.h"
#include "load_object.h"
#include "rico_chunk.h"
#include "rico_cereal.h"
#include "rico_font.h"
#include "primitives.h"
#include "rico_state.h"
#include "test_geom.h"

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
#ifdef RICO_DEBUG
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

    // Backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Multi-sampling
    glEnable(GL_MULTISAMPLE);

    // Alpha-blending
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

int mymain()
{
    enum rico_error err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing third party\n");
    printf("------------------------------------------------------------\n");
    init_stb();
    init_sdl();
    init_gl3w();
    init_opengl();
    init_rico_engine();

    //test_geom();

    enum rico_state state;
    while (1)
    {
        err = state_update(&state);
        if (err) return err;

        if (state == STATE_ENGINE_SHUTDOWN)
            break;

        SDL_GL_SwapWindow(window);
    }

    printf("------------------------------------------------------------\n");
    printf("[MAIN][term] Clean up\n");
    printf("------------------------------------------------------------\n");
    free_glref();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return err;
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
