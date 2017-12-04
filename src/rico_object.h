#ifndef RICO_OBJECT_H
#define RICO_OBJECT_H

#define RICO_OBJ_TYPES(f) \
    f(OBJ_NULL)           \
    f(OBJ_STATIC)         \
    f(OBJ_STRING_WORLD)   \
    f(OBJ_STRING_SCREEN)

enum rico_obj_type {
    RICO_OBJ_TYPES(GEN_LIST)
};
extern const char *rico_obj_type_string[];

struct rico_object {
    struct hnd hnd;
    enum rico_obj_type type;

    // TODO: Refactor into rico_transform
    // TODO: Animation
    struct vec3 trans;
    struct vec3 rot;
    struct vec3 scale;
    struct mat4 transform;
    struct mat4 transform_inverse;

    // TODO: Support multiple meshes and textures
    // TODO: Cache; don't serialize/overwrite when loading
    // TODO: Overwrite *mesh with mesh->uid on save?
    // TODO: Overwrite uid with hash_search(handles, uid) on load?
    struct rico_mesh *mesh;
    struct rico_material *material;

    struct bbox bbox;
};
const u32 RICO_OBJECT_SIZE = sizeof(struct rico_object);

int object_request_by_name(struct rico_object *_obj, const char *name);
int object_create(struct rico_object *_obj, enum rico_persist persist,
                  const char *name, enum rico_obj_type type,
                  struct rico_mesh *mesh, struct rico_material *material,
                  const struct bbox *bbox, bool serialize);
int object_copy(struct rico_object *_obj, struct rico_object *other,
                const char *name);
void object_free(struct rico_object *obj);
void object_free_all(enum rico_persist persist);
void object_bbox_recalculate_all();
void object_bbox_set(struct rico_object *obj, const struct bbox *bbox);
void object_mesh_set(struct rico_object *obj, struct rico_mesh *mesh,
                     const struct bbox *bbox);
void object_mesh_next(struct rico_object *obj);
void object_mesh_prev(struct rico_object *obj);
void object_material_set(struct rico_object *obj,
                         struct rico_material *material);
enum rico_obj_type object_type_get(struct rico_object *obj);
bool object_selectable(struct rico_object *obj);
struct rico_object *object_next(struct rico_object *obj);
struct rico_object *object_prev(struct rico_object *obj);
void object_select(struct rico_object *obj);
void object_deselect(struct rico_object *obj);
void object_trans(struct rico_object *obj, const struct vec3 *v);
void object_trans_set(struct rico_object *obj, const struct vec3 *v);
const struct vec3 *object_trans_get(struct rico_object *obj);
void object_rot(struct rico_object *obj, const struct vec3 *v);
void object_rot_set(struct rico_object *obj, const struct vec3 *v);
const struct vec3 *object_rot_get(struct rico_object *obj);
void object_rot_x(struct rico_object *obj, float deg);
void object_rot_x_set(struct rico_object *obj, float deg);
void object_rot_y(struct rico_object *obj, float deg);
void object_rot_y_set(struct rico_object *obj, float deg);
void object_rot_z(struct rico_object *obj, float deg);
void object_rot_z_set(struct rico_object *obj, float deg);
void object_scale(struct rico_object *obj, const struct vec3 *v);
void object_scale_set(struct rico_object *obj, const struct vec3 *v);
const struct vec3 *object_scale_get(struct rico_object *obj);
const struct mat4 *object_transform_get(struct rico_object *obj);
bool object_collide_ray(float *_dist, struct rico_object *obj,
                        const struct ray *ray);
bool object_collide_ray_type(struct rico_object *_obj, float *_dist,
                             enum rico_obj_type type, const struct ray *ray);
void object_render(struct rico_object *obj, const struct camera *camera);
void object_render_type(enum rico_obj_type type,
                        const struct program_default *prog,
                        const struct camera *camera);
int object_print(struct rico_object *obj, enum rico_string_slot slot);
void object_to_string(struct rico_object *obj, char *buf, int buf_count);
//SERIAL(object_serialize_0);
//DESERIAL(object_deserialize_0);

#endif // RICO_OBJECT_H
