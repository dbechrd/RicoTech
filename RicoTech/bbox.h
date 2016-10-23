#ifndef BBOX_H
#define BBOX_H

#include "geom.h"
#include "program.h"
#include "mesh.h"
#include <GL/gl3w.h>
//#include <malloc.h>
//#include <math.h>
//#include <stdio.h>
//#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////

struct bbox {
    GLuint vao;
    GLuint vbos[2];
    struct program_bbox *prog;

    //TODO: No reason to store vertices
    struct vec4 vertices[8];
    struct col4 color;
};

const struct bbox *make_bbox(struct vec4 p0, struct vec4 p1);
const struct bbox *make_bbox_color(struct vec4 p0, struct vec4 p1,
                                   struct col4 color);
const struct bbox *make_bbox_mesh(const struct mesh_vertex *verts, int count);
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
