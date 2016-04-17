#ifndef RECT_H
#define RECT_H

#include <GL/gl3w.h>
#include "geom.h"

typedef struct Rect {
    Vec3 pos;
    GLfloat w, h;

    Vec4 vertices[4];
    GLuint vao;
} Rect;

Rect *Rect_create(const Vec3 bottom_left, const GLfloat w, const GLfloat h);
void Rect_destroy(Rect *rect);

void Rect_move(Rect *rect, const GLfloat x, const GLfloat y, const GLfloat z);
void Rect_resize(Rect *rect, const GLfloat w, const GLfloat h);
void Rect_render(Rect *rect);

#endif