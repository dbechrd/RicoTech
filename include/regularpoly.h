#ifndef REGULARPOLY_H
#define REGULARPOLY_H

#include "geom.h"
#include <stdbool.h>

struct regularpoly {
    struct vec4 pos;
    GLfloat radius;

    struct vec4 *vertices;
    unsigned int count;
    GLuint vao;
    bool dirty_vao;
};

int init_regularpoly_program();
struct regularpoly *make_regularpoly(struct vec4 center, GLfloat radius,
                                unsigned int vertex_count);
void free_regularpoly(struct regularpoly *);
void set_regularpoly_pos(struct regularpoly *, GLfloat x, GLfloat y, GLfloat z,
                         bool rebuild);
void render_regularpoly(struct regularpoly *);

#endif