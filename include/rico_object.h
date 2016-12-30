#ifndef RICO_OBJ_H
#define RICO_OBJ_H

#include "geom.h"
#include "rico_mesh.h"
#include "rico_uid.h"

struct program_default;

#define RICO_OBJ_TYPES(f)   \
    f(OBJ_NULL)             \
    f(OBJ_DEFAULT)          \
    f(OBJ_STRING_WORLD)     \
    f(OBJ_STRING_SCREEN)    \
    f(OBJ_ALL)

enum rico_obj_type {
    RICO_OBJ_TYPES(GEN_LIST)
};
extern const char *rico_uid_type_string[];

int rico_object_init(u32 pool_size);
int object_create(u32 *_handle, const char *name, enum rico_obj_type type,
                  u32 mesh, u32 texture, const struct bbox *bbox,
                  bool serialize);
int object_copy(u32 *_handle, u32 handle, const char *name);
void object_mesh_set(u32 handle, u32 mesh, const struct bbox *bbox);
void object_texture_set(u32 handle, u32 texture);
void object_free(u32 handle);
void object_free_all();
struct rico_object *object_fetch(u32 handle);
u32 object_next(u32 handle);
u32 object_prev(u32 handle);
enum rico_obj_type object_type_get(u32 handle);
void object_select(u32 handle);
void object_deselect(u32 handle);
void object_trans(u32 handle, const struct vec3 *v);
void object_trans_set(u32 handle, const struct vec3 *v);
const struct vec3 *object_trans_get(u32 handle);
void object_rot(u32 handle, const struct vec3 *v);
void object_rot_set(u32 handle, const struct vec3 *v);
const struct vec3 *object_rot_get(u32 handle);
void object_rot_x(u32 handle, float deg);
void object_rot_x_set(u32 handle, float deg);
void object_rot_y(u32 handle, float deg);
void object_rot_y_set(u32 handle, float deg);
void object_rot_z(u32 handle, float deg);
void object_rot_z_set(u32 handle, float deg);
void object_scale(u32 handle, const struct vec3 *v);
void object_scale_set(u32 handle, const struct vec3 *v);
const struct vec3 *object_scale_get(u32 handle);
const struct mat4 *object_transform_get(u32 handle);
bool object_collide_ray(u32 handle, const struct ray *ray, float *_dist);
u32 object_collide_ray_type(u32 *_handle, u32 count, enum rico_obj_type type,
                            const struct ray *ray, float *_dist);
void object_render(u32 handle, const struct program_default *prog,
                   const struct camera *camera);
void object_render_type(enum rico_obj_type type,
                        const struct program_default *prog,
                        const struct camera *camera);
int object_print(u32 handle, enum rico_string_slot slot);
char *object_serialize_str(u32 handle);
int object_serialize_0(const void *handle, const struct rico_file *file);
int object_deserialize_0(void *_handle, const struct rico_file *file);

struct rico_pool *object_pool_get_unsafe();
void object_pool_set_unsafe(struct rico_pool *pool);

#endif // RICO_OBJ_H