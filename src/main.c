#include "../src/rico.c"

#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 2

static SDL_Window *window = NULL;
static SDL_GLContext context = NULL;

static inline void init_stb()
{
    stbi_set_flip_vertically_on_load(1);
}

static inline void init_murmurhash3()
{
    // key=RicoTech, seed=0
    MurmurHash3_seed = 3533902173;

    const char key[] = "This is a MurmurHash3 test key";
    uint32_t hash;
    MurmurHash3_x86_32(key, sizeof(key) - 1, &hash);
    printf("Key: %s\n", key);
    printf("Hash: %u\n", hash);
}

static int sdl_gl_attrib(SDL_GLattr attr, int value)
{
    int sdl_err = SDL_GL_SetAttribute(attr, value);
    if (sdl_err < 0)
    {
        return RICO_ERROR(ERR_SDL_INIT, "SDL_GL_SetAttribute %d error: %s",
                          attr, SDL_GetError());
    }
    return SUCCESS;
}

static int init_sdl()
{
    enum rico_error err;
    int sdl_err;

    sdl_err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (sdl_err < 0) {
        return RICO_ERROR(ERR_SDL_INIT, "SDL_Init error: %s", SDL_GetError());
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
        return RICO_FATAL(ERR_SDL_INIT, "SDL_CreateWindow error: %s",
                          SDL_GetError());
    }

    // Create GL context
    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        return RICO_FATAL(ERR_SDL_INIT, "SDL_GL_CreateContext error: %s",
                          SDL_GetError());
    }

    // Disable V-sync
    sdl_err = SDL_GL_SetSwapInterval(1); //Default on
    if (sdl_err < 0) {
        return RICO_ERROR(ERR_SDL_INIT, "SDL_GL_SetSwapInterval error: %s",
                          SDL_GetError());
    }
    else
    {
        int swap = SDL_GL_GetSwapInterval();
        fprintf(stdout, "VSync = %s\n", (swap) ? "Enabled" : "Disabled");
    }

    return SUCCESS;
}

static int init_gl3w(int major, int minor)
{
    int init = gl3wInit();
    if (init)
        return RICO_FATAL(ERR_GL3W_INIT, "gl3wInit failed with code %d", init);

    fprintf(stdout, "OpenGL = \"%s\"\nGLSL = \"%s\"\n", glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));

    if (!gl3wIsSupported(major, minor))
        return RICO_FATAL(ERR_GL3W_INIT, "OpenGL 3.2 not supported");

    return SUCCESS;
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

#include "3rdparty\CacheLineSize.h"

int mymain()
{
    enum rico_error err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing third party\n");
    printf("------------------------------------------------------------\n");
    size_t cacheSize = CacheLineSize();
    printf("Cache line size: %d bytes\n", cacheSize);
    init_stb();
    init_murmurhash3();
    err = init_sdl();
    if (err) goto cleanup;
    err = init_gl3w(GL_VERSION_MAJOR, GL_VERSION_MINOR);
    if (err) goto cleanup;
    init_opengl();
    init_rico_engine();

    //test_geom();

    enum rico_state state;
    while (1)
    {
        err = state_update(&state);
        if (err) break;

        if (state == STATE_ENGINE_SHUTDOWN)
            break;

        SDL_GL_SwapWindow(window);
    }

cleanup:
    printf("------------------------------------------------------------\n");
    printf("[MAIN][term] Clean up\n");
    printf("------------------------------------------------------------\n");
    if (context) SDL_GL_DeleteContext(context);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    return err;
}

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    // Don't report fatal errors again when ALL_ERRORS_FATAL flag is set
#if RICO_DEBUG_ALL_ERRORS_FATAL
    enum rico_error err = mymain();
#else
    enum rico_error err = RICO_FATAL(mymain(), "Top-level generic error");
#endif

    // Hack: SDL_main is stupid and ignores my return value, force exit code
    if (err) exit(err);
    return 0;
}