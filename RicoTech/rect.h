#ifndef RECT_H
#define RECT_H

#include <stdbool.h>
#include <GL/gl3w.h>
#include "geom.h"

struct rect {
    struct vec4 pos;
    GLfloat w, h;

    struct vec4 vertices[4];
    GLuint vao;
    bool dirty_vao;
};

int set_rect_program(const char *vertex_shader_filename,
                     const char *fragment_shader_filename);

struct rect *make_rect(struct vec4 bottom_left, GLfloat w, GLfloat h);
void free_rect(struct rect *);

void set_rect_size(struct rect *, GLfloat w, GLfloat h, bool rebuild);
void set_rect_pos(struct rect *, GLfloat x, GLfloat y, GLfloat z, bool rebuild);
void resize_rect(struct rect *, GLfloat w, GLfloat h, bool rebuild);
void render_rect(struct rect *);

#endif