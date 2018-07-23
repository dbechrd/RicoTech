#ifndef RICO_OBJECT_H
#define RICO_OBJECT_H

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

    // TODO: Replace bbox with aabb
    //struct RICO_aabb aabb;
    //struct RICO_aabb aabb_world;
    struct RICO_bbox bbox;
    struct RICO_bbox bbox_world;
    struct RICO_obb obb;
    struct sphere sphere;

    // TODO: Refactor into physics?
    struct vec3 acc;
    struct vec3 vel;
    bool resting;
    bool collide_sphere;
    bool collide_aabb;
    bool collide_obb;

    bool selected;
    bool select_ignore;

    pkid mesh_id;
    pkid material_id;
};

extern void RICO_object_bbox_set(struct RICO_object *obj,
                                 const struct RICO_bbox *bbox);
extern void RICO_object_mesh_set(struct RICO_object *obj, pkid mesh_id);
extern void RICO_object_material_set(struct RICO_object *obj, pkid material_id);
extern void RICO_object_trans(struct RICO_object *obj, const struct vec3 *v);
extern const struct vec3 *RICO_object_trans_get(struct RICO_object *obj);
extern void RICO_object_trans_set(struct RICO_object *obj,
                                  const struct vec3 *v);

/* CLEANUP: Stuff
#define RICO_OBJECT(name) struct name { struct RICO_object rico;

#define RICO_EVENT_OBJECT(name) void name(struct RICO_object *obj)
typedef RICO_EVENT_OBJECT(RICO_event_object_def);
extern RICO_event_object_def *RICO_event_object_interact;
*/
#endif
