#ifndef GLREF_H
#define GLREF_H

#include "geom.h"

extern struct vec4 view_scale;
extern struct vec4 view_trans;
extern struct vec4 view_rot;

void init_glref();
void update_glref(GLfloat dt);
void render_glref();
void free_glref();

#endif // !GLREF_H