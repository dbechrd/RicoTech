#ifndef GLREF_H
#define GLREF_H

#include "geom.h"
#include "camera.h"

void init_glref();
void select_next_obj();
void update_glref(GLfloat dt, bool ambient_light);
void render_glref();
void free_glref();

#endif // !GLREF_H