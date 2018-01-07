#ifndef BBOX_H
#define BBOX_H

struct rico_mesh;

////////////////////////////////////////////////////////////////////////////////
// TODO: Move this to primitives

// TODO: Implement reuse of data for bounding boxes.. no need to initialize
//       an entirely new vao/vbo for every bbox.
// TODO: Don't serialize vao/vbo!

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct bbox
{
    //struct hnd hnd;

    //GLuint vao;
    //GLuint vbos[2];
    //struct program_primitive *prog;

    struct vec3 p;
    struct vec4 color;

    bool wireframe;
};

struct rico_vertex;

void bbox_init(struct bbox *bbox, struct vec3 p, struct vec4 color);
void bbox_init_mesh(struct bbox *bbox, struct rico_mesh *mesh,
                    struct vec4 color);
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
    return !(a->p.x < -b->p.x ||
             b->p.x < -a->p.x ||
             a->p.y < -b->p.y ||
             b->p.y < -a->p.y ||
             a->p.z < -b->p.z ||
             b->p.z < -a->p.z);
}

#endif