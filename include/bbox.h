#ifndef BBOX_H
#define BBOX_H

#include "geom.h"
#include "program.h"
#include <GL/gl3w.h>
//#include <malloc.h>
//#include <math.h>
//#include <stdio.h>
//#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////

struct mesh_vertex;

struct bbox {
    GLuint vao;
    GLuint vbos[2];
    struct program_bbox *prog;

    struct vec4 p0, p1;
    struct col4 color;

    bool wireframe;
};

int bbox_init(struct bbox *bbox, struct vec4 p0, struct vec4 p1,
              struct col4 color);
int bbox_init_mesh(struct bbox *bbox, const struct mesh_vertex *verts,
                   int count, struct col4 color);
void bbox_render(const struct bbox *box, const struct mat4 *model_matrix);
void bbox_render_color(const struct bbox *box, const struct mat4 *model_matrix,
                       const struct col4 color);
bool bbox_intersects(const struct bbox *a, const struct bbox *b);

static inline void free_bbox(struct bbox **bbox)
{
    free(*bbox);
    *bbox = NULL;
}

#endif // BBOX_H
