#ifndef RICO_COLLISION_H
#define RICO_COLLISION_H

//#include "const.h"
//#include "geom.h"
//#include "bbox.h"

bool collide_ray_bbox(const struct ray *ray, const struct bbox *bbox,
                      const struct mat4 *transform);
bool collide_ray_obb(const struct ray *r, const struct bbox *bbox,
                     const struct mat4 *model_matrix,
                     const struct mat4 *model_matrix_inv, float *_dist);

#endif // RICO_COLLISION_H