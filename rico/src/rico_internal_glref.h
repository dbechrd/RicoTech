#ifndef RICO_INTERNAL_GLREF_H
#define RICO_INTERNAL_GLREF_H

#define WIDGET_ACTIONS(f) \
    f(WIDGET_NONE)        \
    f(WIDGET_TRANSLATE_X) \
    f(WIDGET_TRANSLATE_Y) \
    f(WIDGET_TRANSLATE_Z)

enum widget_action { WIDGET_ACTIONS(GEN_LIST) };
const char *widget_action_string[];

struct program_pbr *prog_pbr;
struct program_primitive *prog_prim;
struct program_text *prog_text;

static void editor_init();
static void edit_object_create(struct pack *pack);
static void edit_bbox_reset_all();
static void edit_object_select(struct rico_object *obj, bool force);
static void edit_object_next();
static void edit_object_prev();
static void edit_print_object();
static void edit_translate(struct camera *camera, const struct vec3 *offset);
static void edit_rotate(const struct quat *offset);
static void edit_scale(const struct vec3 *offset);
static void edit_mesh_next();
static void edit_mesh_prev();
static void edit_bbox_reset();
static void edit_duplicate();
static void edit_delete();
static void edit_mouse_pressed();
static void edit_mouse_move();
static void edit_mouse_released();
static void edit_render(struct camera *camera);
static void free_glref();

#endif