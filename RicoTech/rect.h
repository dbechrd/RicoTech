#ifndef RECT_H
#define RECT_H

#include <GL/gl3w.h>
#include "geom.h"

typedef struct Rect {
    GLfloat x, y, z;
    GLfloat w, h;
    
    Vec4 vertices[4];
    //int indices[6];

    //GLuint program;
    GLuint vao;
} Rect;

Rect *Rect_create(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h);
void Rect_destroy(Rect *rect);

void Rect_move(Rect *rect, GLfloat x, GLfloat y, GLfloat z);
void Rect_resize(Rect *rect, GLfloat w, GLfloat h);
void Rect_render(Rect *rect);

#endif