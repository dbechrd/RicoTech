#ifndef RICO_OBJECT_H
#define RICO_OBJECT_H

extern void RICO_object_bbox_set(struct RICO_object *obj,
                                 const struct RICO_aabb *aabb);
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
