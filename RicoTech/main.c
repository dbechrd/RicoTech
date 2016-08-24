#ifdef __APPLE__
#define __gl_h_
//#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

#include "const.h"
#include "geom.h"
#include "util.h"
#include "glref.h"

#include <GL/gl3w.h>
#include <SDL/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_TGA
#include "stb_image.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static SDL_Window *window = NULL;
static SDL_GLContext context = NULL;

static inline void init_stb()
{
    stbi_set_flip_vertically_on_load(1);
}

static void init_sdl()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

#if _DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    //                    SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //Default on
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); //Default 16 bits

    //Initialize window
    window = SDL_CreateWindow("Test Window",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_W, SCREEN_H, SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    //Turn off V-sync
    SDL_GL_SetSwapInterval(0); //Default on
    const char *error = SDL_GetError();
    if (error)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
    }

    {
        int swap = SDL_GL_GetSwapInterval();
        fprintf(stdout, "V-sync = %s\n", (swap) ? "Enabled" : "Disabled");
    }
}

static void init_gl3w()
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

    glEnable(GL_DEPTH_TEST); //Enable depth testing. Default off.
    glDepthFunc(GL_LEQUAL);  //Depth test mode. Default GL_LESS.
}

static void rico_init()
{
    init_stb();
    init_sdl();
    init_gl3w();
    init_opengl();
}

int main(int argc, char *argv[])
{
    rico_init();

    //TODO: Use Unity test framework (http://www.throwtheswitch.org/unity)
    //http://www.drdobbs.com/testing/unit-testing-in-c-tools-and-conventions/240156344
    //run_tests();

    init_glref();

    //Human walk speed empirically found to be 33 steps in 20 seconds. That
    //is approximately 1.65 steps per second. At 60 fps, that is 0.0275 steps
    //per frame. Typical walking stride is ~0.762 meters (30 inches). Distance
    //travelled per frame (60hz) is 0.762 * 0.0275 = 0.020955 ~= 0.021

    //TODO: This assumes 1/60th second (60fps), this should be 1.65 * dt.
    GLfloat view_trans_delta = 1.65f;
    struct vec4 view_trans_vel = { 0.0f, 0.0f, 0.0f, 1.0f };
    bool sprint = false;
    bool fly = false;

    GLfloat view_scale_delta = 0.1f;
    struct vec4 view_scale_vel = { 0.0f, 0.0f, 0.0f, 1.0f };
    
    GLfloat view_rot_delta = 0.1f;

    GLenum lineMode = GL_FILL;

    bool mouse_lock = true;
    SDL_SetRelativeMouseMode(mouse_lock);

    bool pause = false;
    bool quit = false;
    GLuint time = SDL_GetTicks();
    SDL_Event windowEvent;
    while (!quit)
    {
        while (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
            else if (windowEvent.type == SDL_MOUSEMOTION)
            {
                int dx = windowEvent.motion.xrel;
                int dy = windowEvent.motion.yrel;
                //fprintf(stderr, "dx: %d dy: %d\n", dx, dy);

                view_camera.rot.x += dy * view_rot_delta;
                view_camera.rot.y += dx * view_rot_delta;
            }
            else if (windowEvent.type == SDL_KEYDOWN && !windowEvent.key.repeat)
            {
                if (windowEvent.key.keysym.sym == SDLK_LCTRL)
                {
                    sprint = true;
                }

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
                else if (windowEvent.key.keysym.sym == SDLK_d)
                {
                    view_trans_vel.x -= view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_w)
                {
                    view_trans_vel.z += view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_s)
                {
                    view_trans_vel.z -= view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_f)
                {
                    fly = !fly;
                }
                else if (windowEvent.key.keysym.sym == SDLK_p)
                {
                    pause = true;
                }
                else if (windowEvent.key.keysym.sym == SDLK_m)
                {
                    mouse_lock = !mouse_lock;
                    SDL_SetRelativeMouseMode(mouse_lock);
                }
                else if (windowEvent.key.keysym.sym == SDLK_1)
                {
                    lineMode = GL_LINE;
                }
                else if (windowEvent.key.keysym.sym == SDLK_2)
                {
                    lineMode = GL_FILL;
                }
            }
            else if (windowEvent.type == SDL_KEYUP)
            {
                if (windowEvent.key.keysym.sym == SDLK_LCTRL)
                {
                    sprint = false;
                }

                if (windowEvent.key.keysym.sym == SDLK_q)
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
                else if (windowEvent.key.keysym.sym == SDLK_d)
                {
                    view_trans_vel.x += view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_w)
                {
                    view_trans_vel.z -= view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_s)
                {
                    view_trans_vel.z += view_trans_delta;
                }
                else if (windowEvent.key.keysym.sym == SDLK_p)
                {
                    pause = false;
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

        GLuint newTime = SDL_GetTicks();
        GLfloat dt = (float)(newTime - time) / 1000.0f;
        time = newTime;

        GLfloat cos_head = cosf(view_camera.rot.y * (float)M_PI / 180.0f);
        GLfloat sin_head = sinf(view_camera.rot.y * (float)M_PI / 180.0f);

        GLfloat view_dx = (view_trans_vel.x * cos_head
                         - view_trans_vel.z * sin_head) * dt;
        GLfloat view_dy = view_trans_vel.y * dt;
        GLfloat view_dz = (view_trans_vel.z * cos_head
                         + view_trans_vel.x * sin_head) * dt;

        if (fly)
        {
            view_dx *= 5.0f;
            view_dy *= 5.0f;
            view_dz *= 5.0f;
        }

        if (sprint)
        {
            view_dx *= 10.0f;
            view_dz *= 10.0f;
            
            //Debug: Sprint vertically.. yeah.. what?
            view_dy *= 10.0f;
        }

        view_camera.trans.x += view_dx;
        view_camera.trans.y += view_dy;
        view_camera.trans.z += view_dz;

        if (pause)
        {
            //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // Very Important:    *** DEFINITELY ***
            //                    DO NOT REMOVE this variable
            //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            int asdfwaagasdfsahhgfd = 5;
            //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }

        view_camera.scale.x += view_scale_vel.x * dt;
        view_camera.scale.y += view_scale_vel.y * dt;
        view_camera.scale.z += view_scale_vel.z * dt;

        //TODO: Gravity

        //TODO: Collision

        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        //glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, lineMode);

        update_glref(dt);
        render_glref();

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

///////////////////////////////////////////////////////////////////

//TODO: Make buffer somewhere else.. idk where yet
static GLuint orig_make_buffer(const GLenum target, const void *buffer_data, const GLsizei buffer_size)
{
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}
