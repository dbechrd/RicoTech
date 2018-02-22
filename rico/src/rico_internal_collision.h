#ifndef RICO_INTERNAL_COLLISION_H
#define RICO_INTERNAL_COLLISION_H

static bool collide_ray_plane(struct vec3 *_contact, const struct ray *ray,
                              const struct vec3 *p, const struct vec3 *n);
static bool collide_ray_bbox(float *_t, const struct ray *ray,
                             const struct RICO_bbox *RICO_bbox,
                             const struct mat4 *transform);
static bool collide_ray_obb(float *_dist, const struct ray *r,
                            const struct RICO_bbox *RICO_bbox,
                            const struct mat4 *model_matrix);

#endif