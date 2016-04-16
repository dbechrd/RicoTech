#include <stdlib.h>
#include <stdio.h>
#include "rect.h"
#include "util.h"
#include "structs.h"

//static const int rect_indices[6] = { 1, 3, 2, 3, 4, 2 };

static GLuint rect_program;
static GLint rect_prog_attrib_position;

static char rect_init = 0;

static void Rect_rebuildVAO(Rect *rect);

static char Rect_init()
{
    GLuint rect_vshader = make_shader(
        GL_VERTEX_SHADER,
        "poly.v.glsl"
        );
    if (!rect_vshader)
    {
        fprintf(stderr, "Rect: Failed to make vertex shader.\n");
        return 0;
    }

    GLuint rect_fshader = make_shader(
        GL_FRAGMENT_SHADER,
        "poly.f.glsl"
        );
    if (!rect_fshader)
    {
        fprintf(stderr, "Rect: Failed to make fragment shader.\n");
        return 0;
    }

    rect_program = make_program(
        rect_vshader,
        rect_fshader
        );
    if (!rect_program)
    {
        fprintf(stderr, "Rect: Failed to make program.\n");
        return 0;
    }

    rect_prog_attrib_position = glGetAttribLocation(rect_program, "position");
    if (rect_prog_attrib_position == -1)
    {
        fprintf(stderr, "Rect: Failed to get atrribute location for position.\n");
        return 0;
    }

    return 1;
}

Rect *Rect_create(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h)
{
    if (!rect_init && !Rect_init())
    {
        fprintf(stderr, "Rect: Init failed.\n");
        return NULL;
    }

    Rect *rect = (Rect*)malloc(sizeof(Rect));

    rect->x = x;
    rect->y = y;
    rect->z = z;

    rect->w = w;
    rect->h = h;

    rect->vao = 0;
    Rect_rebuildVAO(rect);

    return rect;
}

void Rect_destroy(Rect *rect)
{
    if (rect->vao)
    {
        glDeleteVertexArrays(1, &rect->vao);
    }

    free(rect);
    rect = NULL;
}

void Rect_move(Rect *rect, GLfloat x, GLfloat y, GLfloat z)
{
    rect->x = x;
    rect->y = y;
    rect->z = z;

    Rect_rebuildVAO(rect);
}

void Rect_resize(Rect *rect, GLfloat w, GLfloat h)
{
    rect->w = w;
    rect->h = h;

    Rect_rebuildVAO(rect);
}

static void Rect_rebuildVAO(Rect *rect)
{
    if (rect->vao)
    {
        glDeleteVertexArrays(1, &rect->vao);
    }

    rect->vertices[0] = (Vec4) { rect->x          , rect->y          , rect->z, 1.0f };
    rect->vertices[1] = (Vec4) { rect->x + rect->w, rect->y          , rect->z, 1.0f };
    rect->vertices[2] = (Vec4) { rect->x          , rect->y + rect->h, rect->z, 1.0f };
    rect->vertices[3] = (Vec4) { rect->x + rect->w, rect->y + rect->h, rect->z, 1.0f };

    glGenVertexArrays(1, &rect->vao);
    glBindVertexArray(rect->vao);

    GLuint rect_vbo[2]; // = make_buffer(GL_ARRAY_BUFFER, rect->vertices, sizeof(Vec4)*rect->count);

    glGenBuffers(2, &rect_vbo[0]);

    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect->vertices), rect->vertices, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect_vbo[1]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rect_indices), rect_indices, GL_STATIC_DRAW);

    //(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    glVertexAttribPointer(rect_prog_attrib_position, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(rect_prog_attrib_position);

    glBindVertexArray(0);
    glDeleteBuffers(1, &rect_vbo[0]);
    glDeleteBuffers(1, &rect_vbo[1]);
}

void Rect_render(Rect *rect)
{
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glUseProgram(rect_program);
    glBindVertexArray(rect->vao);

    //(GLenum mode, GLint first, GLsizei count)
    glDrawArrays(
        GL_TRIANGLE_STRIP,
        0,
        4
    );

    glBindVertexArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}