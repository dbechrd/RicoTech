#ifndef BBOX_H
#define BBOX_H

#include "geom.h"
#include "program.h"

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
void bbox_free_mesh(struct bbox *bbox);
void bbox_render(const struct bbox *box, const struct mat4 *proj_matrix,
                 const struct mat4 *view_matrix,
                 const struct mat4 *model_matrix);
void bbox_render_color(const struct bbox *box, const struct mat4 *proj_matrix,
                       const struct mat4 *view_matrix,
                       const struct mat4 *model_matrix,
                       const struct col4 color);

static inline bool bbox_intersects(const struct bbox *a, const struct bbox *b)
{
    return !(a->p1.x < b->p0.x ||
             b->p1.x < a->p0.x ||
             a->p1.y < b->p0.y ||
             b->p1.y < a->p0.y ||
             a->p1.z < b->p0.z ||
             b->p1.z < a->p0.z);
}

#endif // BBOX_H
