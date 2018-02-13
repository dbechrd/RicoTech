#include "rico_internal.h"
#include "gl3w.c"

#define DLB_MATH_IMPLEMENTATION
#define DLB_MATH_PRINT
#include "dlb_math.h"

#include "const.c"
#include "util.c"
#include "rico_hash.c"
#include "rico_audio.c"

#include "rico_hnd.c"
#include "shader.c"
#include "program.c"
#include "camera.c"
#include "bbox.c"

#include "rico_cereal.c"
#include "rico_material.c"
#include "rico_object.c"
#include "glref.c"

#include "primitives.c"
#include "regularpoly.c"
//#include "rico_chunk.c"
#include "rico_collision.c"
#include "rico_convert.c"
#include "rico_font.c"
//#include "rico_light.c"
#include "rico_mesh.c"
#include "rico_physics.c"
//#include "rico_pool.c"
#include "pack_builder.c"

#include <time.h>
#include "rico_state.c"

#include "rico_string.c"
#include "rico_texture.c"
#include "rico_file.c"
#if RICO_DEBUG
    #include "tests.c"
#endif

//#include "CacheLineSize.c"
//#include "main_nuke.c"
#include "3rdparty/MurmurHash3.c"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

global SDL_Window *window = 0;
global SDL_GLContext gl_context = 0;

int RIC_init(int argc, char* argv[]);
int RIC_run();
void RIC_quit();

internal inline void init_stb();
internal inline void init_murmurhash3();
internal int sdl_gl_attrib(SDL_GLattr attr, int value);
internal int init_sdl();
internal int init_gl3w(int major, int minor);
internal void init_opengl();

int RIC_init(int argc, char* argv[])
{
	UNUSED(argc);
	UNUSED(argv);

    // Random sanity checks; I just want to know when these change.
    RICO_ASSERT(sizeof(struct obj_property) < 256);

    enum rico_error err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing third party\n");
    printf("------------------------------------------------------------\n");
    //size_t cacheSize = CacheLineSize();
    //printf("Cache line size: %d bytes\n", cacheSize);
    init_stb();
    init_murmurhash3();

    pack_build_default();

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
	if (err) printf("Error: %s", rico_error_string[err]);
	return err;
}

int RIC_run()
{
	enum rico_error err = SUCCESS;

	while (!SDL_QuitRequested())
	{
		////////////////////////////////////////////////////////////////////////
		// TODO: How do I check for SDL resize window events?
		////////////////////////////////////////////////////////////////////////
		/*
		// Resize OpenGL viewport
		else if (event.type == SDL_WINDOWEVENT_SIZE)
		{
		glViewport(0, 0, event.window.event.size.width,
		event.window.event.size.height);
		*handled = true;
		}
		*/

		err = engine_update();
		if (err) break;

		if (state_get() == STATE_ENGINE_SHUTDOWN)
			break;

		SDL_GL_SwapWindow(window);

#if RICO_DEBUG
		// HACK: Kill some time (a.k.a. save my computer from lighting itself
		//       on fire when VSync is disabled)
		SDL_Delay(3);
#endif
	}

	return err;
}

void RIC_quit()
{
	printf("------------------------------------------------------------\n");
	printf("[MAIN][term] Clean up\n");
	printf("------------------------------------------------------------\n");
	alDeleteSources(1, &audio_source);
	alDeleteBuffers(1, &audio_buffer);
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

internal inline void init_stb()
{
    stbi_set_flip_vertically_on_load(1);
}

internal inline void init_murmurhash3()
{
    // key=RicoTech, seed=0
    MurmurHash3_seed = 3533902173;

    const char key[] = "This is a MurmurHash3 test key";
    u32 hash;
    MurmurHash3_x86_32(key, sizeof(key), &hash);
    printf("Key: %s\n", key);
    printf("Hash: %u\n", hash);
}

internal int sdl_gl_attrib(SDL_GLattr attr, int value)
{
    int sdl_err = SDL_GL_SetAttribute(attr, value);
    if (sdl_err < 0)
    {
        return RICO_ERROR(ERR_SDL_INIT, "SDL_GL_SetAttribute %d error: %s",
                          attr, SDL_GetError());
    }
    return SUCCESS;
}

internal int init_sdl()
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

	err = sdl_gl_attrib(SDL_GL_CONTEXT_FLAGS,
						SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    err = sdl_gl_attrib(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    //                  SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    if (err) return err;

    err = sdl_gl_attrib(SDL_GL_CONTEXT_MAJOR_VERSION, 3); if (err) return err;
    err = sdl_gl_attrib(SDL_GL_CONTEXT_MINOR_VERSION, 2); if (err) return err;

    // Create window
    window = SDL_CreateWindow("RicoTech", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL) {
        return RICO_FATAL(ERR_SDL_INIT, "SDL_CreateWindow error: %s",
                          SDL_GetError());
    }

    // Create GL context
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        return RICO_FATAL(ERR_SDL_INIT, "SDL_GL_CreateContext error: %s",
                          SDL_GetError());
    }

    // Log default VSync state
    int swap = SDL_GL_GetSwapInterval();
    fprintf(stdout, "VSync = %s\n", (swap) ? "Enabled" : "Disabled");

    return SUCCESS;
}

internal int init_gl3w(int major, int minor)
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

internal void init_opengl()
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
    glEnable(GL_CULL_FACE); // Backface culling
    glCullFace(GL_BACK);

    // Multi-sampling
    glEnable(GL_MULTISAMPLE);

    // Alpha-blending
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}
