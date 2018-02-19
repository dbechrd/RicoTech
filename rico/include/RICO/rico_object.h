#ifndef RICO_OBJECT_H
#define RICO_OBJECT_H

#define RICO_OBJECT_TYPES(f) \
    f(RICO_OBJECT_TYPE_NULL)           \
    f(RICO_OBJECT_TYPE_TERRAIN)        \
    f(RICO_OBJECT_TYPE_STRING_SCREEN)

enum rico_object_type
{
    RICO_OBJECT_TYPES(GEN_LIST)
    RICO_OBJECT_TYPE_START = 16
};
extern const char *rico_obj_type_string[];

struct rico_transform
{
    struct vec3 position;
    struct quat orientation;
    struct vec3 scale;
    struct mat4 matrix;
    struct mat4 matrix_inverse;
};

#if 0
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
#endif

struct rico_object
{
    struct uid uid;
    u32 type;
    struct rico_transform xform;
    struct bbox bbox;
    pkid mesh_pkid;
    pkid material_pkid;

    //struct obj_property props[PROP_COUNT];
};

#define RICO_EVENT_OBJECT(name) void name(struct rico_object *obj)
typedef RICO_EVENT_OBJECT(RICO_event_object);

RICO_event_object *RICO_event_object_interact;

#endif
