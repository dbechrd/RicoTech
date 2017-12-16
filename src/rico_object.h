#ifndef RICO_OBJECT_H
#define RICO_OBJECT_H

#define RICO_OBJ_TYPES(f) \
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
    struct pool_id mesh_id;
    struct pool_id material_id;

    struct bbox bbox;
};
extern struct pool_id RICO_DEFAULT_OBJECT;

int object_init(struct rico_object *object, const char *name,
                enum rico_obj_type type, struct pool_id mesh_id,
                struct pool_id material_id, const struct bbox *bbox);
int object_copy(struct rico_object *object, struct rico_object *other,
                const char *name);
int object_free(struct rico_object *object);
void object_free_all(struct rico_chunk *chunk);
void object_bbox_recalculate_all(struct rico_chunk *chunk);
void object_bbox_set(struct rico_object *object, const struct bbox *bbox);
void object_mesh_set(struct rico_object *object, struct pool_id mesh_id);
void object_material_set(struct rico_object *object,
                         struct pool_id material_id);
bool object_selectable(struct rico_object *object);
void object_select(struct rico_object *object);
void object_deselect(struct rico_object *object);
void object_trans(struct rico_object *object, const struct vec3 *v);
void object_trans_set(struct rico_object *object, const struct vec3 *v);
const struct vec3 *object_trans_get(struct rico_object *object);
void object_rot(struct rico_object *object, const struct vec3 *v);
void object_rot_set(struct rico_object *object, const struct vec3 *v);
const struct vec3 *object_rot_get(struct rico_object *object);
void object_rot_x(struct rico_object *object, float deg);
void object_rot_x_set(struct rico_object *object, float deg);
void object_rot_y(struct rico_object *object, float deg);
void object_rot_y_set(struct rico_object *object, float deg);
void object_rot_z(struct rico_object *object, float deg);
void object_rot_z_set(struct rico_object *object, float deg);
void object_scale(struct rico_object *object, const struct vec3 *v);
void object_scale_set(struct rico_object *object, const struct vec3 *v);
const struct vec3 *object_scale_get(struct rico_object *object);
const struct mat4 *object_transform_get(struct rico_object *object);
bool object_collide_ray(float *_dist, struct rico_object *object,
                        const struct ray *ray);
bool object_collide_ray_type(struct rico_chunk *chunk,
                             struct rico_object **_object, float *_dist,
                             enum rico_obj_type type, const struct ray *ray);
void object_render(struct rico_object *object, const struct camera *camera);
void object_render_type(struct rico_chunk *chunk, enum rico_obj_type type,
                        const struct program_default *prog,
                        const struct camera *camera);
int object_print(struct rico_object *object);
void object_to_string(struct rico_object *object, char *buf, int buf_count);

#endif // RICO_OBJECT_H
