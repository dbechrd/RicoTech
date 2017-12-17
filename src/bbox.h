#ifndef BBOX_H
#define BBOX_H

////////////////////////////////////////////////////////////////////////////////
// TODO: Move this to primitives

// TODO: Implement reuse of data for bounding boxes.. no need to initialize
//       an entirely new vao/vbo for every bbox.
// TODO: Don't serialize vao/vbo!

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct bbox {
    //struct hnd hnd;

    //GLuint vao;
    //GLuint vbos[2];
    //struct program_primitive *prog;

    struct vec3 p[2];
    struct col4 color;

    bool wireframe;
};

int bbox_init(struct bbox *bbox, struct vec3 p0, struct vec3 p1,
              struct col4 color);
int bbox_init_mesh(struct bbox *bbox, const struct mesh_vertex *verts,
                   int count, struct col4 color);
#if 0
void bbox_free_mesh(struct bbox *bbox);
void bbox_render(const struct bbox *box, const struct mat4 *model_matrix);
void bbox_render_color(const struct bbox *box, const struct mat4 *model_matrix,
                       const struct col4 color);
SERIAL(bbox_serialize_0);
DESERIAL(bbox_deserialize_0);
#endif

internal inline bool bbox_intersects(const struct bbox *a, const struct bbox *b)
{
    return !(a->p[1].x < b->p[0].x ||
             b->p[1].x < a->p[0].x ||
             a->p[1].y < b->p[0].y ||
             b->p[1].y < a->p[0].y ||
             a->p[1].z < b->p[0].z ||
             b->p[1].z < a->p[0].z);
}

#endif // BBOX_H
