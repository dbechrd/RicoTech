#ifndef PRIMITIVES_H
#define PRIMITIVES_H

extern struct pool_id PRIM_MESH_BBOX;
extern struct pool_id PRIM_MESH_SPHERE;

struct prim_vertex
{
    struct vec3 pos;
    struct vec4 col;
};

int prim_init();
void prim_draw_line(const struct vec3 p0, const struct vec3 p1,
                    const struct mat4 *matrix, const struct vec4 color);
void prim_draw_ray(const struct ray *ray, const struct mat4 *matrix,
                   const struct vec4 color);
void prim_draw_quad(const struct quad *quad, const struct mat4 *matrix,
                    const struct vec4 *color);
void prim_draw_plane(const struct vec3 *p, const struct vec3 *n,
                     const struct mat4 *matrix, const struct vec4 *color);
void prim_draw_bbox(const struct bbox *bbox, const struct mat4 *matrix,
                    const struct vec4 *color);
void prim_draw_sphere(const struct sphere *sphere, const struct vec4 *color);
void prim_free();

#endif