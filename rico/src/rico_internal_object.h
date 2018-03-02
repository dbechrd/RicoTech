#ifndef RICO_INTERNAL_OBJECT_H
#define RICO_INTERNAL_OBJECT_H

#include "RICO/rico_object.h"

static void object_delete(struct RICO_object *obj);
static struct RICO_object *object_copy(u32 pack, struct RICO_object *other,
                                       const char *name);
static void object_bbox_recalculate(struct RICO_object *obj);
static void object_bbox_recalculate_all(u32 id);
static bool object_selectable(struct RICO_object *rico);
static void object_select(struct RICO_object *rico);
static void object_deselect(struct RICO_object *rico);
static void object_transform_update(struct RICO_object *rico);
static void object_trans(struct RICO_object *rico, const struct vec3 *v);
static void object_trans_set(struct RICO_object *rico, const struct vec3 *v);
static const struct vec3 *object_trans_get(struct RICO_object *rico);
static void object_rot(struct RICO_object *rico, const struct quat *q);
static void object_rot_set(struct RICO_object *rico, const struct quat *q);
static const struct quat *object_rot_get(struct RICO_object *rico);
static void object_scale(struct RICO_object *rico, const struct vec3 *v);
static void object_scale_set(struct RICO_object *rico, const struct vec3 *v);
static const struct vec3 *object_scale_get(struct RICO_object *rico);
static const struct mat4 *object_matrix_get(struct RICO_object *rico);
static bool object_collide_ray(float *_dist, struct RICO_object *rico,
                               const struct ray *ray);
static bool object_collide_ray_type(u32 id, struct RICO_object **_object,
                                    float *_dist, const struct ray *ray);
static void object_interact(struct RICO_object *obj);
static void object_render(struct pack *pack, const struct camera *camera);
static void object_render_ui(struct pack *pack);
static void object_render_all(r64 alpha, struct camera *camera);
static void object_print(struct RICO_object *obj);

#endif