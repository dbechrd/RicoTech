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
    hash_key mesh_key;
    u32 mesh;

    u32 material;

    struct bbox bbox;
};
extern const u32 RICO_OBJECT_SIZE;

#define RICO_OBJ_TYPES(f)   \
    f(OBJ_NULL)             \
    f(OBJ_STATIC)           \
    f(OBJ_STRING_WORLD)     \
    f(OBJ_STRING_SCREEN)

enum rico_obj_type {
    RICO_OBJ_TYPES(GEN_LIST)
};
//extern const char *rico_obj_type_string[];

int object_create(u32 *_handle, const char *name, enum rico_obj_type type,
                  hash_key mesh_key, u32 material, const struct bbox *bbox,
                  bool serialize);
int object_copy(u32 *_handle, u32 handle, const char *name);
void object_free(u32 handle);
void object_free_all();
void object_bbox_recalculate_all();
void object_bbox_set(u32 handle, const struct bbox *bbox);
int object_mesh_set(u32 handle, hash_key mesh_key, const struct bbox *bbox);
void object_mesh_next(u32 handle);
void object_mesh_prev(u32 handle);
void object_material_set(u32 handle, u32 material);
enum rico_obj_type object_type_get(u32 handle);
bool object_selectable(u32 handle);
u32 object_next(u32 handle);
u32 object_prev(u32 handle);
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
u32 object_collide_ray_type(enum rico_obj_type type, const struct ray *ray,
                            u32 count, u32 *_handle, float *_dist, u32 *_first);
void object_render_type(enum rico_obj_type type,
                        const struct program_default *prog,
                        const struct camera *camera);
int object_print(u32 handle, enum rico_string_slot slot);
void object_to_string(u32 handle, char *buf, int buf_count);
SERIAL(object_serialize_0);
DESERIAL(object_deserialize_0);

#endif // RICO_OBJECT_H