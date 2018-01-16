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

    struct vec3 min;
    struct vec3 max;
    struct vec4 color;

    bool wireframe;
};

void bbox_init(struct bbox *bbox, struct vec3 min, struct vec3 max,
               struct vec4 color);
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
    return !(a->max.x < b->min.x ||
             b->max.x < a->min.x ||
             a->max.y < b->min.y ||
             b->max.y < a->min.y ||
             a->max.z < b->min.z ||
             b->max.z < a->min.z);
}

#endif