#ifndef RICO_COLLISION_H
#define RICO_COLLISION_H

bool collide_ray_plane(const struct ray *ray, const struct vec3 *p,
                       const struct vec3 *n, struct vec3 *_contact);
bool collide_ray_bbox(const struct ray *ray, const struct bbox *bbox,
                      const struct mat4 *transform);
bool collide_ray_obb(float *_dist, const struct ray *r, const struct bbox *bbox,
                     const struct mat4 *model_matrix,
                     const struct mat4 *model_matrix_inv);

#endif