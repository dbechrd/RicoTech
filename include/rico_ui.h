#ifndef RIC_UI_STATE_H
#define RIC_UI_STATE_H

extern struct ric_ui_hud *ric_ui_hud();
extern struct ric_ui_head *ric_ui_line_break(struct ric_ui_hud *parent);
extern struct ric_ui_button *ric_ui_button(struct ric_ui_hud *parent);
extern struct ric_ui_label *ric_ui_label(struct ric_ui_hud *parent);
extern struct ric_ui_progress *ric_ui_progress(struct ric_ui_hud *parent);
extern bool ric_ui_layout(struct ric_ui_element *element, s32 x, s32 y,
                          s32 max_w, s32 max_h);
extern void ric_ui_draw(struct ric_ui_element *element, s32 x, s32 y);

#endif