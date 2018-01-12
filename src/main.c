#include "rico.c"

global SDL_Window *window = 0;
global SDL_GLContext gl_context = 0;
internal ALCdevice *audio_device = 0;
internal ALCcontext *audio_context = 0;
internal ALuint audio_source;
internal ALuint audio_buffer;

internal inline void init_stb();
internal inline void init_murmurhash3();
internal int sdl_gl_attrib(SDL_GLattr attr, int value);
internal int init_sdl();
internal int init_gl3w(int major, int minor);
internal void init_opengl();
internal int init_openal();

int mymain()
{
    // Random sanity checks; I just want to know when these change.
    RICO_ASSERT(sizeof(struct obj_property) < 64);

    enum rico_error err;

    printf("------------------------------------------------------------\n");
    printf("[MAIN][init] Initializing third party\n");
    printf("------------------------------------------------------------\n");
    //size_t cacheSize = CacheLineSize();
    //printf("Cache line size: %d bytes\n", cacheSize);
    init_stb();
    init_murmurhash3();

    pack_build_all();
    pack_transient = pack_init("pack_transient", 0, MB(4));
    pack_frame = pack_init("pack_frame", 0, 0);
    err = pack_load("packs/default.pak", &pack_default);
    err = pack_load("packs/alpha.pak", &pack_active);

    err = init_sdl();
    if (err) goto cleanup;
    err = init_gl3w(GL_VERSION_MAJOR, GL_VERSION_MINOR);
    if (err) goto cleanup;
    init_opengl();
    init_openal();
    init_rico_engine();

#if RICO_DEBUG
    run_tests();
#endif

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

        err = state_update();
        if (err) break;

        if (state_get() == STATE_ENGINE_SHUTDOWN)
            break;

        SDL_GL_SwapWindow(window);
        // HACK: Kill some time (a.k.a. save my computer from lighting itself
        //       on fire when VSync is disabled)
        SDL_Delay(3);
    }

cleanup:
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
    return err;
}

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    enum rico_error err;

    // Don't report fatal errors again when ALL_ERRORS_FATAL flag is set
#if RICO_DEBUG_ALL_ERRORS_FATAL
    err = mymain();
#else
    err = RICO_FATAL(mymain(), "Top-level generic error");
#endif
    if (err) printf("Error: %s", rico_error_string[err]);

    // Hack: SDL_main is stupid and ignores my return value, force exit code
    if (err) exit(err);
    return err;
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

    err = sdl_gl_attrib(SDL_GL_RED_SIZE, 8);          if (err) return err;
    err = sdl_gl_attrib(SDL_GL_GREEN_SIZE, 8);          if (err) return err;
    err = sdl_gl_attrib(SDL_GL_BLUE_SIZE, 8);          if (err) return err;
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

internal int init_openal()
{
    ALenum err;

    const char *devices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    const char *s = devices;
    while (*s)
    {
        printf("Device: %s\n", s);
        s += strlen(s) + 1;
    }

    audio_device = alcOpenDevice(NULL);
    if (!audio_device)
        return RICO_FATAL(ERR_OPENAL_INIT, "Failed to open audio device");
    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    ALCint attrlist[] = { ALC_FREQUENCY, 44100, 0 };
    audio_context = alcCreateContext(audio_device, attrlist);
    if (!audio_context)
        return RICO_FATAL(ERR_OPENAL_INIT, "Failed to create audio context");
    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    if (!alcMakeContextCurrent(audio_context))
        return RICO_FATAL(ERR_OPENAL_INIT, "Failed to activate audio context");
    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    alListener3f(AL_POSITION, 0.0, 1.5, 0.0);
    alListener3f(AL_VELOCITY, 0.0, 0.0, 0.0);
    struct vec3 fwd_up[2] = {
        VEC3(0.0f, 0.0f, -1.0f),
        VEC3(0.0f, 1.0f,  0.0f)
    };
    alListenerfv(AL_ORIENTATION, (float *)fwd_up);
    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    alGenSources(1, &audio_source);
    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);
    alSourcef(audio_source, AL_PITCH, 1.0f);
    alSourcef(audio_source, AL_GAIN, 1.0f);
    alSourcei(audio_source, AL_LOOPING, AL_TRUE);
    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    struct vec3 src_pos = VEC3(0.0f, 0.0f, -1.0f);
    alSourcefv(audio_source, AL_POSITION, (float *)&src_pos);
    alSourcefv(audio_source, AL_VELOCITY, (float *)&VEC3_ZERO);
    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    alGenBuffers(1, &audio_buffer);
    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    // TODO: Load from file
    #define SAMPLE_RATE 44100
    const u32 AMPLITUDE = 15000;
    s16 buf[SAMPLE_RATE];

    //const double TWO_PI = 6.28318;
    //const double c4 = 261.63 / 44100;
    //const double e4 = 329.63 / 44100;
    //const double g4 = 392.00 / 44100;
    const double ring1 = 350.0 / 44100;
    const double ring2 = 440.0 / 44100;
    double x1 = 0;
    double x2 = 0;
    double x3 = 0;
    for (unsigned i = 0; i < SAMPLE_RATE; ++i)
    {
        buf[i] = (s16)(AMPLITUDE / 2 * (sin(x1 * M_2PI) + sin(x2 * M_2PI) + sin(x3 * M_2PI)));
        x1 += ring1;
        x2 += ring2;
        //x1 += c4;
        //x2 += e4;
        //x3 += g4;
    }

    /*
    const double TWO_PI = 6.28318;
    const double sample_rate = 44100.0;
    const double frequency = 440.0;
    const double increment = TWO_PI / (44100.0 / 440.0);
    for (u32 i = 0; i < 440; ++i)
    {
        for (double j = 0; j < TWO_PI; j += increment)
        {
            buf[i] = (s16)(AMPLITUDE * sin(j));
        }
    }
    */

    alBufferData(audio_buffer, AL_FORMAT_MONO16, buf, sizeof(buf), SAMPLE_RATE);
    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    //alSourceQueueBuffers(audio_source, 1, &audio_buffer);
    alSourcei(audio_source, AL_BUFFER, audio_buffer);

    ALenum state;
    alGetSourcei(audio_source, AL_SOURCE_STATE, &state);

    alSourcePlay(audio_source);
    alGetSourcei(audio_source, AL_SOURCE_STATE, &state);

    err = alGetError();
    if (err) RICO_ERROR(ERR_OPENAL_INIT, "OpenAL error: %s\n", err);

    return SUCCESS;
}