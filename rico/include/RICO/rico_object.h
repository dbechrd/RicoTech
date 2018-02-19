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

#define RICO_OBJECT(name) struct name { struct rico_object rico;

#define RICO_EVENT_OBJECT(name) void name(struct rico_object *obj)
typedef RICO_EVENT_OBJECT(RICO_event_object);

RICO_event_object *RICO_event_object_interact;

#endif
