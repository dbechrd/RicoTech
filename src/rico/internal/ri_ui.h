#ifndef RICO_INTERNAL_UI_H
#define RICO_INTERNAL_UI_H

static void rico_ui_init();
static void rico_ui_reset();
static void ui_debug_stack_usage();
static void ui_draw_element(struct RICO_ui_element *element, u32 x, u32 y);

#endif