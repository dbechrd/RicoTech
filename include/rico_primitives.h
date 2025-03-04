#ifndef RICO_PRIMITIVES_H
#define RICO_PRIMITIVES_H

extern void ric_prim_draw_line2d(float x0, float y0, float x1, float y1,
                                 const struct vec4 *color);
extern void ric_prim_draw_line(const struct vec3 *p0, const struct vec3 *p1,
                               const struct vec4 *color);
extern void ric_prim_draw_line_xform(const struct vec3 *p0,
                                     const struct vec3 *p1,
                                     const struct vec4 *color,
                                     const struct mat4 *xform);
extern void ric_prim_draw_rect(const struct rect *rect,
                               const struct vec4 *color);
extern void ric_prim_draw_sprite(const struct rect *rect,
                                 const struct ric_sprite *sprite,
                                 const struct vec4 *color);
extern void ric_prim_draw_rect_tex(const struct rect *rect,
                                   const struct vec4 *color, pkid tex_id);
extern void ric_prim_draw_ray(const struct ray *ray, const struct vec4 *color);
extern void ric_prim_draw_ray_xform(const struct ray *ray,
                                    const struct vec4 *color,
                                    const struct mat4 *xform);
extern void ric_prim_draw_quad(const struct quad *quad,
                               const struct vec4 *color);
extern void ric_prim_draw_quad_xform(const struct quad *quad,
                                     const struct vec4 *color,
                                     const struct mat4 *xform);
extern void ric_prim_draw_plane(const struct vec3 *n,
                                const struct vec4 *color);
extern void ric_prim_draw_plane_xform(const struct vec3 *n,
                                      const struct vec4 *color,
                                      const struct mat4 *xform);
extern void ric_prim_draw_aabb(const struct ric_aabb *aabb,
                               const struct vec4 *color);
extern void ric_prim_draw_aabb_xform(const struct ric_aabb *aabb,
                                     const struct vec4 *color,
                                     const struct mat4 *xform);
extern void ric_prim_draw_obb(const struct ric_obb *obb,
                              const struct vec4 *color);
extern void ric_prim_draw_obb_xform(const struct ric_obb *obb,
                                    const struct vec4 *color,
                                    const struct mat4 *xform);
extern void ric_prim_draw_sphere(const struct sphere *sphere,
                                 const struct vec4 *color);
extern void ric_prim_draw_sphere_xform(const struct sphere *sphere,
                                       const struct vec4 *color,
                                       const struct mat4 *xform);

#endif
