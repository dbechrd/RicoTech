#ifdef __APPLE__
#define __gl_h_
//#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

#include <GL/gl3w.h>
#include <SDL/SDL.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "geom.h"
#include "util.h"
#include "structs.h"
#include "rect.h"
#include "bullet.h"

static int make_resources(const char *vertex_shader_file);
static GLuint make_vao();
//static GLuint make_texture(const char *filename);
//static GLuint make_shader(const GLenum type, const char *filename);
//static GLuint make_program(const GLuint vertex_shader, const GLuint fragment_shader);
static void update_timer();
static void render(void);
//void show_info_log(
//    GLuint object,
//    PFNGLGETSHADERIVPROC glGet__iv,
//    PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

void APIENTRY openglCallbackFunction(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *userParam) {

    char *typeStr, *severityStr;

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        typeStr = "ERROR\0";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeStr = "DEPRC\0";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeStr = "UNDEF\0";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        typeStr = "PORT \0";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        typeStr = "PERF \0";
        break;
    case GL_DEBUG_TYPE_OTHER:
        typeStr = "OTHER\0";
        break;
    default:
        typeStr = "?????\0";
        break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        severityStr = "LOW ";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severityStr = "MED ";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        severityStr = "HIGH";
        break;
    default:
        severityStr = "????";
        break;
    }

    fprintf(stderr, "[%s][%s][%d] %s\n", typeStr, severityStr, id, message);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

#if _DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    //SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //SDL_GL_SetSwapInterval(1);

    //Initialize window
    SDL_Window *window = SDL_CreateWindow("Test Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (gl3wInit())
    {
        fprintf(stderr, "gl3wInit failed.\n");
        getchar();
        return 1;
    }

    fprintf(stdout, "OpenGL = \"%s\"\nGLSL = \"%s\"\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    if (!gl3wIsSupported(3, 1))
    {
        fprintf(stderr, "OpenGL 3.2 not supported.\n");
        getchar();
        return 1;
    }

#if _DEBUG
    if (glDebugMessageCallback != NULL)
    {
        fprintf(stdout, "Registered glDebugMessageCallback.\n");
        fprintf(stderr, "[TYPE][SEVERITY][ID] MESSAGE\n");
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglCallbackFunction, NULL);
        GLuint unusedIds = 0;
        glDebugMessageControl(GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DONT_CARE,
            0,
            &unusedIds,
            true);
    }
    else
    {
        fprintf(stderr, "glDebugMessageCallback not available.\n");
    }
#endif

    if (!make_resources(argc >= 2 ? argv[1] : "durian.v.glsl"))
    {
        fprintf(stderr, "Failed to load resources.\n");
        getchar();
        return 1;
    }

    Rect *player = Rect_create(0.0f, 0.0f, 1.0f, 32.0f, 64.0f);
    if (!player)
    {
        return 1;
    }

    Rect *floor = Rect_create(-512.0f, 0.0f, 1.0f, 1024.0f, 1.0f);

    Bullet *bullet = NULL;

    RegularPoly *pentagon = RegularPoly_create(Vec4_create(300.0f, 0.0f, 1.0f, 1.0f), 5);
    
    GLuint pentagon_vshader = make_shader(
        GL_VERTEX_SHADER,
        "poly.v.glsl"
        );
    SDL_assert(pentagon_vshader != 0);

    GLuint pentagon_fshader = make_shader(
        GL_FRAGMENT_SHADER,
        "poly.f.glsl"
        );
    SDL_assert(pentagon_fshader != 0);

    GLuint pentagon_program = make_program(
        pentagon_vshader,
        pentagon_fshader
        );
    SDL_assert(pentagon_program != 0);

    GLint penta_prog_position = glGetAttribLocation(pentagon_program, "position");
    if (penta_prog_position == -1)
    {
        fprintf(stderr, "Failed to get atrribute location for position.\n");
        return 1;
    }

    GLuint pentagon_vao;

    glGenVertexArrays(1, &pentagon_vao);
    glBindVertexArray(pentagon_vao);

    GLuint pentagon_vbo; // = make_buffer(GL_ARRAY_BUFFER, pentagon->vertices, sizeof(Vec4)*pentagon->count);

    glGenBuffers(1, &pentagon_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, pentagon_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pentagon->vertices[0])*pentagon->count, pentagon->vertices, GL_STATIC_DRAW);

    //(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    glVertexAttribPointer(penta_prog_position, 4, GL_FLOAT, GL_FALSE, 0, 0);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pentagon_elements);
    glEnableVertexAttribArray(penta_prog_position);

    glBindVertexArray(0);
    glDeleteBuffers(1, &pentagon_vbo);

    GLfloat player_dxvel = 10.0f;
    GLfloat player_xvel = 0.0f;

    GLfloat player_dyvel = 20.0f;
    GLfloat player_yvel = 0.0f;

    char jump_pressed = 0;
    char shoot_pressed = 0;

    SDL_Event windowEvent;
    while (true)
    {
        if (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT)
            {
                break;
            }
            if (windowEvent.type == SDL_KEYUP)
            {
                if (windowEvent.key.keysym.sym == SDLK_a
                 || windowEvent.key.keysym.sym == SDLK_LEFT)
                {
                    player_xvel -= -player_dxvel;
                }
                if (windowEvent.key.keysym.sym == SDLK_d
                 || windowEvent.key.keysym.sym == SDLK_RIGHT)
                {
                    player_xvel -= player_dxvel;
                }
                if (windowEvent.key.keysym.sym == SDLK_w
                 || windowEvent.key.keysym.sym == SDLK_UP)
                {
                    jump_pressed = 0;
                }
                if (windowEvent.key.keysym.sym == SDLK_SPACE)
                {
                    shoot_pressed = 0;
                }
                if (windowEvent.key.keysym.sym == SDLK_ESCAPE)
                {
                    break;
                }
            }
            if (windowEvent.type == SDL_KEYDOWN && !windowEvent.key.repeat)
            {
                if (windowEvent.key.keysym.sym == SDLK_a
                 || windowEvent.key.keysym.sym == SDLK_LEFT)
                {
                    player_xvel += -player_dxvel;
                }
                if (windowEvent.key.keysym.sym == SDLK_d
                 || windowEvent.key.keysym.sym == SDLK_RIGHT)
                {
                    player_xvel += player_dxvel;
                }
                if (windowEvent.key.keysym.sym == SDLK_w
                 || windowEvent.key.keysym.sym == SDLK_UP)
                {
                    jump_pressed = 1;
                }
                if (windowEvent.key.keysym.sym == SDLK_SPACE)
                {
                    shoot_pressed = 1;
                }
            }

            //TODO: Handle resize event
            /*if (windowEvent.type == SDL_WINDOWEVENT_SIZE)
            {
                glViewport(0, 0, windowEvent.window.event..size.width, event.size.height);
                break;
            }*/
        }


        update_timer();

        if (jump_pressed && player->y == 0.0f)
        {
            player_yvel += player_dyvel;
        }

        player_yvel -= 1.0f;
        player->y += player_yvel;

        if (player->y < 0.0f)
        {
            player_yvel = 0.0f;
            player->y = 0.0f;
        }

        if (shoot_pressed && bullet == NULL)
        {
            Rect *rect = Rect_create(
                player->x + (player->w / 2.0f),
                player->y + (player->h / 2.0f),
                player->z,
                4.0f, 2.0f);
            bullet = Bullet_create(rect);
            bullet->vel.x = 20.0f;
        }

        if (bullet != NULL)
        {
            if (bullet->rect->x < -512.0f || bullet->rect->x > 512.0f)
            {
                Bullet_destroy(bullet);
                bullet = NULL;
            }
            else
            {
                Bullet_update(bullet);
            }
        }

        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //render();
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glUseProgram(pentagon_program);
        glBindVertexArray(pentagon_vao);

        //(GLenum mode, GLint first, GLsizei count)
        glDrawArrays(
            GL_TRIANGLE_FAN,
            0,
            pentagon->count
        );

        glBindVertexArray(0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        Rect_move(player, player->x + player_xvel, player->y, player->z);

        Rect_render(floor);
        Rect_render(player);
        if (bullet != NULL)
        {
            Bullet_render(bullet);
        }

        SDL_GL_SwapWindow(window);
    }

    RegularPoly_destroy(pentagon);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

static int make_resources(const char *vertex_shader_file)
{
    g_resources.textures[0] = make_texture("hello1.tga");
    g_resources.textures[1] = make_texture("hello2.tga");

    if (g_resources.textures[0] == 0 || g_resources.textures[1] == 0)
        return 0;

    g_resources.vertex_shader = make_shader(
        GL_VERTEX_SHADER,
        vertex_shader_file
    );
    if (g_resources.vertex_shader == 0)
        return 0;

    g_resources.fragment_shader = make_shader(
        GL_FRAGMENT_SHADER,
        "durian.f.glsl"
    );
    if (g_resources.fragment_shader == 0)
        return 0;

    g_resources.program = make_program(
        g_resources.vertex_shader,
        g_resources.fragment_shader
    );
    if (g_resources.program == 0)
        return 0;

    int getUniformLocations = 1;

    g_resources.uniforms.timer
        = glGetUniformLocation(g_resources.program, "timer");
    if (g_resources.uniforms.timer == -1)
    {
        fprintf(stderr, "Failed to get uniform location for timer.\n");
        getUniformLocations = 0;
    }

    g_resources.uniforms.textures[0]
        = glGetUniformLocation(g_resources.program, "textures[0]");
    if (g_resources.uniforms.textures[0] == -1)
    {
        fprintf(stderr, "Failed to get uniform location for textures[0].\n");
        getUniformLocations = 0;
    }

    g_resources.uniforms.textures[1]
        = glGetUniformLocation(g_resources.program, "textures[1]");
    if (g_resources.uniforms.textures[1] == -1)
    {
        fprintf(stderr, "Failed to get uniform location for textures[1].\n");
        getUniformLocations = 0;
    }

    g_resources.attributes.position
        = glGetAttribLocation(g_resources.program, "position");
    if (g_resources.attributes.position == -1)
    {
        fprintf(stderr, "Failed to get atrribute location for position.\n");
        getUniformLocations = 0;
    }

    if (getUniformLocations == 0)
        return 0;

    g_resources.vao = make_vao();

    return 1;
}

static GLuint make_vao()
{
    GLuint vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(NUM_BUFFERS, g_resources.vbos);

    glBindBuffer(GL_ARRAY_BUFFER, g_resources.vbos[VBO_POSITION]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertices), g_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.vbos[VBO_POSITION_ELEM]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(g_resources.attributes.position);

    //(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    glVertexAttribPointer(g_resources.attributes.position, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);

    glBindVertexArray(0);

    //TODO: Figure out when it makes sense to delete buffers. These are global for testing,
    //        so they never go out of scope; therefore, it doesn't matter that they don't get deleted.
    //glDeleteBuffers(2, g_resources.vbos);

    return vao;
}

static GLuint make_buffer_old(const GLenum target, const void *buffer_data, const GLsizei buffer_size)
{
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

static void update_timer()
{
    g_resources.timer = (GLfloat)SDL_GetTicks() / 1000.0f; //sinf((float)milliseconds * 0.001f) * 1.0f + 1.0f;
}

static void render(void)
{
    glUseProgram(g_resources.program);

    glUniform1f(g_resources.uniforms.timer, g_resources.timer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[0]);
    glUniform1i(g_resources.uniforms.textures[0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[1]);
    glUniform1i(g_resources.uniforms.textures[1], 1);

    glBindVertexArray(g_resources.vao);

    //(GLenum mode, GLsizei count, GLenum type, const void *indices);
    glDrawElements(
        GL_TRIANGLE_STRIP,
        4,
        GL_UNSIGNED_SHORT,
        (void*)0
    );

    glBindVertexArray(0);
    glUseProgram(0);
}
