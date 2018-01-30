#ifndef RICO_COLLISION_H
#define RICO_COLLISION_H

bool collide_ray_plane(struct vec3 *_contact, const struct ray *ray,
                       const struct vec3 *p, const struct vec3 *n);
bool collide_ray_bbox(float *_t, const struct ray *ray, const struct bbox *bbox,
                      const struct mat4 *transform);
bool collide_ray_obb(float *_dist, const struct ray *r, const struct bbox *bbox,
                     const struct mat4 *model_matrix,
                     const struct mat4 *model_matrix_inv);

#endif