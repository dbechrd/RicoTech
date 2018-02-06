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

void editor_init();
void edit_object_create(struct pack *pack);
void edit_bbox_reset_all();
void edit_object_select(struct rico_object *obj, bool force);
void edit_object_next();
void edit_object_prev();
void edit_print_object();
void edit_translate(struct camera *camera, const struct vec3 *offset);
void edit_rotate(const struct vec3 *offset);
void edit_scale(const struct vec3 *offset);
void edit_mesh_next();
void edit_mesh_prev();
void edit_bbox_reset();
void edit_duplicate();
void edit_delete();
void edit_mouse_pressed();
void edit_mouse_move();
void edit_mouse_released();
void edit_render(struct camera *camera);
void free_glref();

#endif