#ifndef BBOX_H
#define BBOX_H

#include "geom.h"
#include "rico_uid.h"
#include "rico_cereal.h"

////////////////////////////////////////////////////////////////////////////////

struct mesh_vertex;
struct camera;

struct bbox {
    struct rico_uid uid;

    GLuint vao;
    GLuint vbos[2];
    struct program_primitive *prog;

    struct vec3 p0;
    struct vec3 p1;
    struct col4 color;

    bool wireframe;
};

int bbox_init(struct bbox *bbox, const char *name, struct vec3 p0,
              struct vec3 p1, struct col4 color);
int bbox_init_mesh(struct bbox *bbox, const char *name,
                   const struct mesh_vertex *verts, int count,
                   struct col4 color);
void bbox_free_mesh(struct bbox *bbox);
void bbox_render(const struct bbox *box, const struct camera *camera,
                 const struct mat4 *model_matrix);
void bbox_render_color(const struct bbox *box, const struct camera *camera,
                       const struct mat4 *model_matrix,
                       const struct col4 color);
int bbox_serialize_0(const void *handle, const struct rico_file *file);
int bbox_deserialize_0(void *_handle, const struct rico_file *file);

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
