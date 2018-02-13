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

#define RICO_PROP_TYPES(f) \
    f(PROP_NULL)           \
    f(PROP_TRANSFORM)      \
    f(PROP_MESH)           \
    f(PROP_TEXTURE)        \
    f(PROP_MATERIAL)       \
    f(PROP_BBOX)           \
    f(PROP_LIGHT_DIR)      \
    f(PROP_LIGHT_POINT)    \
    f(PROP_LIGHT_SPOT)     \
    f(PROP_LIGHT_SWITCH)   \
    f(PROP_AUDIO_SWITCH)   \
    f(PROP_GAME_BUTTON)    \
    f(PROP_COUNT)

enum obj_prop_type
{
    RICO_PROP_TYPES(GEN_LIST)
};
extern const char *rico_prop_type_string[];

struct light_switch
{
    pkid light_id;
    bool state;
};

struct audio_switch
{
    pkid audio_id;
    bool state;
};

struct game_button
{
    pkid button_id;
    bool state;
};

// TODO: This is really dumb.. eventually I'll replace it with: pkid props[]
struct obj_property
{
    enum obj_prop_type type;
    union
    {
        struct rico_transform xform;
        pkid mesh_pkid;
        pkid texture_pkid;
        pkid material_pkid;
        struct bbox bbox;
        struct light_directional light_dir;
        struct light_point light_point;
        struct light_spot light_spot;
        struct light_switch light_switch;
        struct audio_switch audio_switch;
        struct game_button game_button;
    };
};

struct rico_object
{
    struct uid uid;
    enum rico_obj_type type;

    struct obj_property props[PROP_COUNT];
};

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
void object_render_all(struct camera *camera);
void object_print(struct rico_object *obj);

#endif