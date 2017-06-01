#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#define RICO_PRIMITIVES(f)  \
    f(PRIM_SEGMENT)         \
    f(PRIM_RAY)             \
    f(PRIM_COUNT)

enum rico_prim {
    RICO_PRIMITIVES(GEN_LIST)
};
//extern const char *rico_prim_string[];

extern u32 PRIM_SPHERE_MESH;
extern hash_key PRIM_SPHERE_MESH_KEY;

struct prim_vertex {
    struct vec3 pos;
    struct col4 col;
};

struct segment {
    struct prim_vertex vertices[2];
};

int prim_init(enum rico_prim prim);
void prim_draw_segment(const struct segment *seg,
                       const struct mat4 *model_matrix, struct col4 color);
void prim_draw_ray(const struct ray *ray, const struct mat4 *model_matrix,
                   struct col4 color);
void prim_draw_sphere(const struct sphere *sphere, const struct col4 *color);
void prim_free(enum rico_prim prim);

#endif // PRIMITIVES_H