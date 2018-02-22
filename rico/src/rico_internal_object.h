#ifndef RICO_INTERNAL_OBJECT_H
#define RICO_INTERNAL_OBJECT_H

#include "RICO/rico_object.h"

static void object_delete(struct rico_object *obj);
struct rico_object *object_copy(struct pack *pack, struct rico_object *other,
                                const char *name);
static void object_bbox_recalculate(struct rico_object *obj);
static void object_bbox_recalculate_all(struct pack *pack);
bool object_selectable(struct rico_object *rico);
static void object_select(struct rico_object *rico);
static void object_deselect(struct rico_object *rico);
static void object_transform_update(struct rico_object *rico);
static void object_trans(struct rico_object *rico, const struct vec3 *v);
static void object_trans_set(struct rico_object *rico, const struct vec3 *v);
const struct vec3 *object_trans_get(struct rico_object *rico);
static void object_rot(struct rico_object *rico, const struct quat *q);
static void object_rot_set(struct rico_object *rico, const struct quat *q);
const struct quat *object_rot_get(struct rico_object *rico);
/*
static void object_rot_x(struct rico_object *object, float deg);
static void object_rot_x_set(struct rico_object *object, float deg);
static void object_rot_y(struct rico_object *object, float deg);
static void object_rot_y_set(struct rico_object *object, float deg);
static void object_rot_z(struct rico_object *object, float deg);
static void object_rot_z_set(struct rico_object *object, float deg);
*/
static void object_scale(struct rico_object *rico, const struct vec3 *v);
static void object_scale_set(struct rico_object *rico, const struct vec3 *v);
const struct vec3 *object_scale_get(struct rico_object *rico);
const struct mat4 *object_matrix_get(struct rico_object *rico);
bool object_collide_ray(float *_dist, struct rico_object *rico,
                        const struct ray *ray);
bool object_collide_ray_type(struct pack *pack, struct rico_object **_object,
                             float *_dist, const struct ray *ray);
static void object_interact(struct rico_object *obj);
static void object_render(struct pack *pack, const struct camera *camera);
static void object_render_ui(struct pack *pack);
static void object_render_all(struct camera *camera);
static void object_print(struct rico_object *obj);

#endif
