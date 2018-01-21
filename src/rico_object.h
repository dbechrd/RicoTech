#ifndef RICO_OBJECT_H
#define RICO_OBJECT_H

#define RICO_OBJ_TYPES(f) \
    f(OBJ_NULL)           \
    f(OBJ_TERRAIN)        \
    f(OBJ_STATIC)         \
    f(OBJ_STRING_WORLD)   \
    f(OBJ_STRING_SCREEN)  \
    f(OBJ_LIGHT_POINT)

enum rico_obj_type
{
    RICO_OBJ_TYPES(GEN_LIST)
};
extern const char *rico_obj_type_string[];

struct rico_transform
{
    struct vec3 trans;
    struct vec3 rot;
    struct vec3 scale;
    struct mat4 matrix;
    struct mat4 matrix_inverse;
};

enum obj_prop_type
{
    PROP_MESH_ID,
    PROP_TEXTURE_ID,
    PROP_MATERIAL_ID,
    PROP_LIGHT_DIR,
    PROP_LIGHT_POINT,
    PROP_LIGHT_SPOT,
    PROP_LIGHT_SWITCH,
    PROP_AUDIO_SWITCH,
    PROP_COUNT
};

struct light_switch
{
    u32 light_id;
    bool state;
};

struct audio_switch
{
    u32 audio_id;
    bool state;
};

struct obj_property
{
    enum obj_prop_type type;
    union
    {
        // PROP_MESH
        u32 mesh_id;
        // PROP_TEXTURE
        u32 texture_id;
        // PROP_MATERIAL
        u32 material_id;
        // PROP_LIGHT_DIR
        struct light_directional light_dir;
        // PROP_LIGHT_POINT
        struct light_point light_point;
        // PROP_LIGHT_SPOT
        struct light_spot light_spot;
        // PROP_LIGHT_SWITCH
        struct light_switch light_switch;
        // PROP_AUDIO_SWITCH
        struct audio_switch audio_switch;
    };
};

struct rico_object
{
    u32 id;
    enum rico_obj_type type;
    struct bbox bbox;

    // TODO: Refactor into rico_transform
    // TODO: Animation
    struct rico_transform xform;

    u32 name_offset;
    u32 prop_count;
    u32 props_offset;
};

global const char *object_name(struct rico_object *obj);
global struct obj_property *object_props(struct rico_object *obj);
global struct obj_property *object_prop(struct rico_object *obj,
                                        enum obj_prop_type type);
global void object_delete(struct pack *pack, struct rico_object *obj);
global struct rico_object *object_copy(struct pack *pack,
                                       struct rico_object *other,
                                       const char *name);
void object_bbox_recalculate_all(struct pack *pack);
bool object_selectable(struct rico_object *object);
void object_select(struct rico_object *object);
void object_deselect(struct rico_object *object);
global void object_transform_update(struct rico_object *object);
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
const struct mat4 *object_matrix_get(struct rico_object *object);
bool object_collide_ray(float *_dist, struct rico_object *object,
                        const struct ray *ray);
bool object_collide_ray_type(struct pack *pack, struct rico_object **_object,
                             float *_dist, const struct ray *ray);
void object_interact(struct rico_object *obj);
void object_update(struct rico_object *obj);
void object_render(struct pack *pack, const struct camera *camera);
void object_render_ui(struct pack *pack);
void object_print(struct rico_object *obj);

#endif