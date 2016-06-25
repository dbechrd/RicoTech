#include "rect.h"
#include "util.h"
#include "structs.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

//static const int rect_indices[6] = { 1, 3, 2, 3, 4, 2 };

static GLuint rect_program = 0;
static GLint rect_prog_attrib_position = -1;

int set_rect_program(const char *vertex_shader_filename,
                     const char *fragment_shader_filename)
{
    if (rect_program)
    {
        glDeleteProgram(rect_program);
    }

    rect_program = make_program(vertex_shader_filename,
                                fragment_shader_filename);
    if (!rect_program)
    {
        fprintf(stderr, "rect: Failed to make program.\n");
        return 0;
    }

    rect_prog_attrib_position = glGetAttribLocation(rect_program, "position");
    if (rect_prog_attrib_position == -1)
    {
        fprintf(stderr,
                "rect: Failed to get atrribute location for position.\n");
        return 0;
    }

    return 1;
}

static void rebuild_vao(struct rect *rect)
{
    if (rect->vao)
    {
        glDeleteVertexArrays(1, &rect->vao);
    }

    assert(rect_program);

    rect->vertices[0].x = rect->pos.x;
    rect->vertices[0].y = rect->pos.y;
    rect->vertices[0].z = rect->pos.z;
    rect->vertices[0].w = 1.0f;

    rect->vertices[1].x = rect->pos.x + rect->w;
    rect->vertices[1].y = rect->pos.y;
    rect->vertices[1].z = rect->pos.z;
    rect->vertices[1].w = 1.0f;

    rect->vertices[2].x = rect->pos.x;
    rect->vertices[2].y = rect->pos.y + rect->h,
    rect->vertices[2].z = rect->pos.z;
    rect->vertices[2].w = 1.0f;

    rect->vertices[3].x = rect->pos.x + rect->w;
    rect->vertices[3].y = rect->pos.y + rect->h,
    rect->vertices[3].z = rect->pos.z;
    rect->vertices[3].w = 1.0f;

    glGenVertexArrays(1, &rect->vao);
    glBindVertexArray(rect->vao);

    GLuint rect_vbo[2];
    // = make_buffer(GL_ARRAY_BUFFER, rect->vertices,
    //               sizeof(struct vec4)*rect->count);

    glGenBuffers(2, rect_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect->vertices), rect->vertices, 
                 GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect_vbo[1]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rect_indices), rect_indices,
    //             GL_STATIC_DRAW);

    //(GLuint index, GLint size, GLenum type, GLboolean normalized,
    // GLsizei stride, const void *pointer);
    glVertexAttribPointer(rect_prog_attrib_position, 4, GL_FLOAT, GL_FALSE,
                          0, 0);
    glEnableVertexAttribArray(rect_prog_attrib_position);

    glBindVertexArray(0);
    glDeleteBuffers(1, &rect_vbo[0]);
    glDeleteBuffers(1, &rect_vbo[1]);

    rect->dirty_vao = false;
}

static void init_rect(struct rect *rect, struct vec4 bottom_left, GLfloat w,
                      GLfloat h)
{
    rect->pos = bottom_left;

    rect->w = w;
    rect->h = h;

    rect->vao = 0;
    rect->dirty_vao = true;
    rebuild_vao(rect);
}

struct rect *make_rect(struct vec4 bottom_left, GLfloat w, GLfloat h)
{
    struct rect *rect = malloc(sizeof(struct rect));
    if (!rect)
        return NULL;

    init_rect(rect, bottom_left, w, h);

    return rect;
}

void free_rect(struct rect *rect)
{
    if (rect->vao)
    {
        glDeleteVertexArrays(1, &rect->vao);
    }

    free(rect);
    rect = NULL;
}

void set_rect_size(struct rect *rect, GLfloat w, GLfloat h, bool rebuild)
{
    rect->w = w;
    rect->h = h;

    if (rebuild)
        rebuild_vao(rect);
    else
        rect->dirty_vao = true;
}

void set_rect_pos(struct rect *rect, GLfloat x, GLfloat y, GLfloat z,
                  bool rebuild)
{
    rect->pos.x = x;
    rect->pos.y = y;
    rect->pos.z = z;
    
    if (rebuild)
        rebuild_vao(rect);
    else
        rect->dirty_vao = true;
}

void resize_rect(struct rect *rect, GLfloat w, GLfloat h, bool rebuild)
{
    rect->w = w;
    rect->h = h;
    
    if (rebuild)
        rebuild_vao(rect);
    else
        rect->dirty_vao = true;
}

void render_rect(struct rect *rect)
{
    assert(rect_program);

    if (rect->dirty_vao)
    {
        fprintf(stderr, "rect: Rebuilding dirty VAO at render time.");
        rebuild_vao(rect);
    }

    glUseProgram(rect_program);
    glBindVertexArray(rect->vao);

    //(GLenum mode, GLint first, GLsizei count)
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
}