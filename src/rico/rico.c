#include "gl3w.c"

#include "rico.h"
#include "ri.h"

#define DLB_MEMORY_IMPLEMENTATION
#include "dlb_memory.h"
#define DLB_MATH_PRINT
#define DLB_MATH_IMPLEMENTATION
#include "dlb_math.h"
#define DLB_VECTOR_IMPLEMENTATION
#include "dlb_vector.h"
#define DLB_HASH_IMPLEMENTATION
#define DLB_HASH_TEST
#include "dlb_hash.h"
#define DLB_HEAP_IMPLEMENTATION
#include "dlb_heap.h"
#define DLB_RAND_IMPLEMENTATION
#include "dlb_rand.h"

#include "rico_types.c"
#include "rico_error.c"
#include "rico_arena.c"
#include "rico_stream.c"
#include "rico_util.c"
#include "rico_resource.c"
#include "rico_audio.c"
#include "rico_shader.c"
#include "rico_program.c"
#include "rico_camera.c"
#include "rico_aabb.c"
#include "rico_material.c"
#include "rico_shadow.c"
#include "rico_object.c"
#include "rico_editor.c"
#include "rico_primitives.c"
#include "rico_collision.c"
#include "rico_convert.c"
#include "rico_font.c"
#include "rico_mesh.c"
#include "rico_physics.c"
#include "rico_pack.c"
#include "rico_state.c"
#include "rico_string.c"
#include "rico_texture.c"
#include "rico_file.c"
#include "rico_heiro.c"
#include "rico_ui.c"
#include "rico_cereal.c"
#if RICO_DEBUG
    #include "tests.c"
#endif

#include "cache_line_size.c"
//#include "main_nuke.c"
//#include "MurmurHash3.c"

#define STB_IMAGE_IMPLEMENTATION
#include "misc/stb_image.h"

#include <stdlib.h>
#include "time.h"

static SDL_Window *window = 0;
static SDL_GLContext gl_context = 0;

#if _DEBUG
DLB_ASSERT_HANDLER(handle_assert)
{
    fprintf(stdout,
        "\n---[DLB_ASSERT_HANDLER]---------------------------------------------------------\n"
        "Source file: %s:%d\n\n"
        "%s\n"
        "--------------------------------------------------------------------------------\n",
        filename, line, expr);
    UNUSED(getchar());
    exit(-1);
}
dlb_assert_handler_def *dlb_assert_handler = handle_assert;
#endif

static inline void init_stb();
static inline void init_murmurhash3();
static int sdl_gl_attrib(SDL_GLattr attr, int value);
static int init_sdl();
static int init_gl3w(int major, int minor);
static void init_opengl();
static inline void init_stb()
{
    stbi_set_flip_vertically_on_load(1);
}
static inline void init_murmurhash3()
{
    // key=RicoTech, seed=0
    MurmurHash3_seed = 3533902173;

    const char key[] = "This is a MurmurHash3 test key";
    u32 hash;
    MurmurHash3_x86_32(key, sizeof(key), &hash);
    printf("Key: %s\n", key);
    printf("Hash: %u\n", hash);
}
static int sdl_gl_attrib(SDL_GLattr attr, int value)
{
    int sdl_err = SDL_GL_SetAttribute(attr, value);
    if (sdl_err < 0)
    {
        return RICO_ERROR(RIC_ERR_SDL_INIT, "SDL_GL_SetAttribute %d error: %s",
                          attr, SDL_GetError());
    }
    return RIC_SUCCESS;
}
static int init_sdl()
{
    enum ric_error err;
    int sdl_err;

    sdl_err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (sdl_err < 0) {
        return RICO_ERROR(RIC_ERR_SDL_INIT, "SDL_Init error: %s", SDL_GetError());
    }

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

    // Anti-aliasing
    err = sdl_gl_attrib(SDL_GL_MULTISAMPLEBUFFERS, 1);  if (err) return err;
    err = sdl_gl_attrib(SDL_GL_MULTISAMPLESAMPLES, 16); if (err) return err;
    err = sdl_gl_attrib(SDL_GL_ACCELERATED_VISUAL, 1);  if (err) return err;

	err = sdl_gl_attrib(SDL_GL_CONTEXT_FLAGS,
						SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    err = sdl_gl_attrib(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    //                  SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    if (err) return err;

    err = sdl_gl_attrib(SDL_GL_CONTEXT_MAJOR_VERSION, 3); if (err) return err;
    err = sdl_gl_attrib(SDL_GL_CONTEXT_MINOR_VERSION, 2); if (err) return err;

    // Create window
    bool fullscreen = false;
    if (fullscreen)
    {
        window = SDL_CreateWindow(
            "RicoTech", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI |
            SDL_WINDOW_FULLSCREEN_DESKTOP
        );
        SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    }
    else
    {
        window = SDL_CreateWindow(
            "RicoTech", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL |
            SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
        );
    }

    if (window == NULL) {
        return RICO_FATAL(RIC_ERR_SDL_INIT, "SDL_CreateWindow error: %s",
                          SDL_GetError());
    }

    // Create GL context
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        return RICO_FATAL(RIC_ERR_SDL_INIT, "SDL_GL_CreateContext error: %s",
                          SDL_GetError());
    }

    // Log default VSync state
    int swap = SDL_GL_GetSwapInterval();
    fprintf(stdout, "VSync = %s\n", (swap) ? "Enabled" : "Disabled");

    return RIC_SUCCESS;
}
static int init_gl3w(int major, int minor)
{
    int init = gl3wInit();
    if (init)
        return RICO_FATAL(RIC_ERR_GL3W_INIT, "gl3wInit failed with code %d", init);

    fprintf(stdout, "OpenGL = \"%s\"\nGLSL = \"%s\"\n", glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));

    if (!gl3wIsSupported(major, minor))
        return RICO_FATAL(RIC_ERR_GL3W_INIT, "OpenGL 3.2 not supported");

    return RIC_SUCCESS;
}
static void init_opengl()
{
#if RICO_DEBUG
    {
        GLint gl_max_vertex_attribs;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_max_vertex_attribs);
        printf("GL_MAX_VERTEX_ATTRIBS = %d\n", gl_max_vertex_attribs);
    }

    if (glDebugMessageCallback != NULL)
    {
        fprintf(stdout, "Registered glDebugMessageCallback.\n");

        // TODO: OpenGL DebugMessageCallback format header, useful? Where should
        //       this be written?
        fprintf(stderr, "[TYPE][SEVERITY][ID] MESSAGE\n");

        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglCallbackFunction, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0,
                              true);
    }
    else
    {
        fprintf(stderr, "glDebugMessageCallback not available.\n");
    }
#endif

    // Depth buffer
    glDepthFunc(GL_LEQUAL);  // Default GL_LESS
    glEnable(GL_DEPTH_TEST); // Default off
    glEnable(GL_CULL_FACE);  // Backface culling
    glCullFace(GL_BACK);

    // Multi-sampling
    glEnable(GL_MULTISAMPLE);

    // Alpha-blending
    glBlendEquation(GL_FUNC_ADD);  // GL_FUNC_SUBTRACT w/ green for night vision
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // Gamma correction
    //glEnable(GL_FRAMEBUFFER_SRGB);
}
static void platform_window_swap()
{
    SDL_GL_SwapWindow(window);
}

int ric_init()
{
    enum ric_error err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing third party\n");
    printf("------------------------------------------------------------\n");
    dlb_rand_seed((u32)time(0));
    size_t cacheSize = CacheLineSize();
    printf("Cache line min_size: %d bytes\n", cacheSize);
    init_stb();
    init_murmurhash3();

    err = init_sdl();
    if (err) goto error;
    err = init_gl3w(GL_VERSION_MAJOR, GL_VERSION_MINOR);
    if (err) goto error;
    init_opengl();
    init_openal();
    engine_init();

#if RICO_DEBUG_RUN_TESTS
    run_tests();
#endif

error:
    if (err) printf("Error: %s", ric_error_string[err]);
    return err;
}
void ric_window_size(s32 *w, s32 *h)
{
    SDL_GetWindowSize(window, w, h);
}
void ric_cleanup()
{
    printf("------------------------------------------------------------\n");
    printf("[MAIN][term] Clean up\n");
    printf("------------------------------------------------------------\n");
    alcDestroyContext(audio_context);
    alcCloseDevice(audio_device);

    if (gl_context) SDL_GL_DeleteContext(gl_context);
    if (window) SDL_DestroyWindow(window);

    SDL_Quit();
#if RICO_DEBUG && !RICO_DEBUG_ALL_ERRORS_FATAL
    // Don't report fatal errors again when ALL_ERRORS_FATAL flag is set
    err = RICO_FATAL(err, "Top-level generic error");
#endif
}
