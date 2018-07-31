#ifndef RICO_OBJECT_H
#define RICO_OBJECT_H

extern void ric_object_aabb_set(struct ric_object *obj,
                                const struct ric_aabb *aabb);
extern void ric_object_mesh_set(struct ric_object *obj, pkid mesh_id);
extern void ric_object_material_set(struct ric_object *obj, pkid material_id);
extern void ric_object_trans(struct ric_object *obj, const struct vec3 *v);
extern const struct vec3 *ric_object_trans_get(struct ric_object *obj);
extern void ric_object_trans_set(struct ric_object *obj, const struct vec3 *v);

/* CLEANUP: Stuff
#define RIC_OBJECT(name) struct name { struct ric_object rico;

#define RIC_EVENT_OBJECT(name) void name(struct ric_object *obj)
typedef RIC_EVENT_OBJECT(ric_event_object_def);
extern RIC_event_object_def *ric_event_object_interact;
*/
#endif
