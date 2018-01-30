#ifndef GLREF_H
#define GLREF_H

#define WIDGET_ACTIONS(f) \
    f(WIDGET_NONE)        \
    f(WIDGET_TRANSLATE_X) \
    f(WIDGET_TRANSLATE_Y) \
    f(WIDGET_TRANSLATE_Z)

enum widget_action
{
    WIDGET_ACTIONS(GEN_LIST)
};
extern const char *widget_action_string[];

extern struct program_pbr *prog_pbr;
extern struct program_primitive *prog_prim;
extern struct program_text *prog_text;

void glref_init();
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
void edit_mouse_pressed();
void edit_mouse_move(r32 dx, r32 dy);
void edit_mouse_released();
void glref_update();
void glref_render(struct camera *camera);
void free_glref();

#endif