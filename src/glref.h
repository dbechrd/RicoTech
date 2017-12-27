#ifndef GLREF_H
#define GLREF_H

void init_glref();
internal int init_hardcoded_test_chunk();
void create_obj();
void recalculate_all_bbox();
void select_obj(struct rico_object *obj, bool force);
void select_next_obj();
void select_prev_obj();
void selected_print();
void selected_translate(struct camera *camera, const struct vec3 *offset);
void selected_rotate(const struct vec3 *offset);
void selected_scale(const struct vec3 *offset);
void selected_mesh_next();
void selected_mesh_prev();
void selected_bbox_reset();
void selected_duplicate();
void selected_delete();
void glref_update(r64 dt);
void glref_render(struct camera *camera);
void free_glref();

#endif // !GLREF_H
