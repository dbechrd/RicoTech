#ifndef GLREF_H
#define GLREF_H

extern struct program_pbr *prog_pbr;
extern struct program_primitive *prog_prim;
extern struct program_text *prog_text;

void init_glref();
void create_obj(struct pack *pack);
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
void glref_update();
void glref_render(struct camera *camera);
void free_glref();

#endif