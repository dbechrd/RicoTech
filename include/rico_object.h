#ifndef RICO_OBJECT_H
#define RICO_OBJECT_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_object {
    struct rico_uid uid;
    enum rico_obj_type type;

    //TODO: Refactor into rico_transform
    //TODO: Animation
    struct vec3 trans;
    struct vec3 rot;
    struct vec3 scale;
    struct mat4 transform;
    struct mat4 transform_inverse;

    //TODO: Support multiple meshes and textures
    u32 mesh;
    u32 material;

    struct bbox bbox;
};
const u32 RICO_OBJECT_SIZE = sizeof(struct rico_object);

#define RICO_OBJ_TYPES(f) \
    f(OBJ_NULL)           \
    f(OBJ_STATIC)         \
    f(OBJ_STRING_WORLD)   \
    f(OBJ_STRING_SCREEN)

enum rico_obj_type {
    RICO_OBJ_TYPES(GEN_LIST)
};
extern const char *rico_obj_type_string[];

int object_create(u32 *_handle, enum rico_persist persist, const char *name,
                  enum rico_obj_type type, u32 mesh, u32 material,
                  const struct bbox *bbox, bool serialize);
int object_copy(u32 *_handle, enum rico_persist persist, u32 handle,
                const char *name);
void object_free(enum rico_persist persist, u32 handle);
void object_free_all(enum rico_persist persist);
void object_bbox_recalculate_all(enum rico_persist persist);
void object_bbox_set(enum rico_persist persist, u32 handle,
                     const struct bbox *bbox);
void object_mesh_set(enum rico_persist persist, u32 handle, u32 mesh,
                     const struct bbox *bbox);
void object_mesh_next(enum rico_persist persist, u32 handle);
void object_mesh_prev(enum rico_persist persist, u32 handle);
void object_material_set(enum rico_persist persist, u32 handle, u32 material);
enum rico_obj_type object_type_get(enum rico_persist persist, u32 handle);
bool object_selectable(enum rico_persist persist, u32 handle);
u32 object_next(enum rico_persist persist, u32 handle);
u32 object_prev(enum rico_persist persist, u32 handle);
void object_select(enum rico_persist persist, u32 handle);
void object_deselect(enum rico_persist persist, u32 handle);
void object_trans(enum rico_persist persist, u32 handle, const struct vec3 *v);
void object_trans_set(enum rico_persist persist, u32 handle,
                      const struct vec3 *v);
const struct vec3 *object_trans_get(enum rico_persist persist, u32 handle);
void object_rot(enum rico_persist persist, u32 handle, const struct vec3 *v);
void object_rot_set(enum rico_persist persist, u32 handle,
                    const struct vec3 *v);
const struct vec3 *object_rot_get(enum rico_persist persist, u32 handle);
void object_rot_x(enum rico_persist persist, u32 handle, float deg);
void object_rot_x_set(enum rico_persist persist, u32 handle, float deg);
void object_rot_y(enum rico_persist persist, u32 handle, float deg);
void object_rot_y_set(enum rico_persist persist, u32 handle, float deg);
void object_rot_z(enum rico_persist persist, u32 handle, float deg);
void object_rot_z_set(enum rico_persist persist, u32 handle, float deg);
void object_scale(enum rico_persist persist, u32 handle, const struct vec3 *v);
void object_scale_set(enum rico_persist persist, u32 handle,
                      const struct vec3 *v);
const struct vec3 *object_scale_get(enum rico_persist persist, u32 handle);
const struct mat4 *object_transform_get(enum rico_persist persist, u32 handle);
bool object_collide_ray(float *_dist, enum rico_persist persist, u32 handle,
                        const struct ray *ray);
bool object_collide_ray_type(u32 *_handle, float *_dist,
                             enum rico_obj_type type, const struct ray *ray);
void object_render(enum rico_persist persist, u32 handle,
                   const struct camera *camera);
void object_render_type(enum rico_persist persist, enum rico_obj_type type,
                        const struct program_default *prog,
                        const struct camera *camera);
int object_print(enum rico_persist persist, u32 handle,
                 enum rico_string_slot slot);
void object_to_string(enum rico_persist persist, u32 handle, char *buf,
                      int buf_count);
//SERIAL(object_serialize_0);
//DESERIAL(object_deserialize_0);

#endif // RICO_OBJECT_H