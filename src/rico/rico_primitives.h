#ifndef RICO_PRIMITIVES_H
#define RICO_PRIMITIVES_H

extern void RICO_prim_draw_line(const struct vec3 *p0, const struct vec3 *p1,
                                const struct mat4 *matrix,
                                const struct vec4 *color);
extern void RICO_prim_draw_ray(const struct ray *ray, const struct mat4 *matrix,
                               const struct vec4 *color);
extern void RICO_prim_draw_quad(const struct quad *quad,
                                const struct mat4 *matrix,
                                const struct vec4 *color);
extern void RICO_prim_draw_plane(const struct vec3 *n,
                                 const struct mat4 *matrix,
                                 const struct vec4 *color);
extern void RICO_prim_draw_bbox(const struct RICO_bbox *bbox,
                                const struct mat4 *matrix,
                                const struct vec4 *color);
extern void RICO_prim_draw_obb(const struct RICO_obb *obb,
                               const struct vec4 *color);
extern void RICO_prim_draw_sphere(const struct sphere *sphere,
                                  const struct vec4 *color);

#endif
