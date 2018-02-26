#ifndef RICO_OBJECT_H
#define RICO_OBJECT_H

#define RICO_OBJECT_TYPES(f) \
    f(RICO_OBJECT_TYPE_NULL)           \
    f(RICO_OBJECT_TYPE_TERRAIN)        \
    f(RICO_OBJECT_TYPE_STRING_SCREEN)

enum RICO_object_type
{
    RICO_OBJECT_TYPES(GEN_LIST)
    RICO_OBJECT_TYPE_START = 16
};
extern const char *RICO_obj_type_string[];

struct RICO_transform
{
    struct vec3 position;
    struct quat orientation;
    struct vec3 scale;
    struct mat4 matrix;
    struct mat4 matrix_inverse;
};

struct RICO_object
{
    struct uid uid;
    u32 type;
    struct RICO_transform xform;
    struct RICO_bbox bbox;
    pkid mesh_id;
    pkid material_id;
};

#define RICO_OBJECT(name) struct name { struct RICO_object rico;

#define RICO_EVENT_OBJECT(name) void name(struct RICO_object *obj)
typedef RICO_EVENT_OBJECT(RICO_event_object_def);
extern RICO_event_object_def *RICO_event_object_interact;

#endif
