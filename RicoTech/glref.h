#ifndef GLREF_H
#define GLREF_H

#include "geom.h"

struct camera {
    struct vec4 scale;
    struct vec4 rot;
    struct vec4 trans;
};

extern struct camera view_camera;

void init_glref();
void update_glref(GLfloat dt);
void render_glref();
void free_glref();

#endif // !GLREF_H