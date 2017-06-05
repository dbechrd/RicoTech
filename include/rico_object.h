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
    struct hnd mesh;
    struct hnd material;

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

int object_create(struct hnd *_handle, enum rico_persist persist,
                  const char *name, enum rico_obj_type type, struct hnd mesh,
                  struct hnd material, const struct bbox *bbox,
                  bool serialize);
int object_copy(struct hnd *_handle, struct hnd handle,
                const char *name);
void object_free(struct hnd handle);
void object_free_all(enum rico_persist persist);
void object_bbox_recalculate_all();
void object_bbox_set(struct hnd handle,
                     const struct bbox *bbox);
void object_mesh_set(struct hnd handle, struct hnd mesh,
                     const struct bbox *bbox);
void object_mesh_next(struct hnd handle);
void object_mesh_prev(struct hnd handle);
void object_material_set(struct hnd handle, struct hnd material);
enum rico_obj_type object_type_get(struct hnd handle);
bool object_selectable(struct hnd handle);
struct hnd object_next(struct hnd handle);
struct hnd object_prev(struct hnd handle);
void object_select(struct hnd handle);
void object_deselect(struct hnd handle);
void object_trans(struct hnd handle, const struct vec3 *v);
void object_trans_set(struct hnd handle,
                      const struct vec3 *v);
const struct vec3 *object_trans_get(struct hnd handle);
void object_rot(struct hnd handle, const struct vec3 *v);
void object_rot_set(struct hnd handle,
                    const struct vec3 *v);
const struct vec3 *object_rot_get(struct hnd handle);
void object_rot_x(struct hnd handle, float deg);
void object_rot_x_set(struct hnd handle, float deg);
void object_rot_y(struct hnd handle, float deg);
void object_rot_y_set(struct hnd handle, float deg);
void object_rot_z(struct hnd handle, float deg);
void object_rot_z_set(struct hnd handle, float deg);
void object_scale(struct hnd handle, const struct vec3 *v);
void object_scale_set(struct hnd handle,
                      const struct vec3 *v);
const struct vec3 *object_scale_get(struct hnd handle);
const struct mat4 *object_transform_get(struct hnd handle);
bool object_collide_ray(float *_dist, struct hnd handle,
                        const struct ray *ray);
bool object_collide_ray_type(struct hnd *_handle, float *_dist,
                             enum rico_obj_type type, const struct ray *ray);
void object_render(struct hnd handle,
                   const struct camera *camera);
void object_render_type(enum rico_obj_type type,
                        const struct program_default *prog,
                        const struct camera *camera);
int object_print(struct hnd handle, enum rico_string_slot slot);
void object_to_string(struct hnd handle, char *buf, int buf_count);
//SERIAL(object_serialize_0);
//DESERIAL(object_deserialize_0);

#endif // RICO_OBJECT_H