#ifndef RIC_UI_STATE_H
#define RIC_UI_STATE_H

extern struct RICO_ui_hud *RICO_ui_hud();
extern struct RICO_ui_head *RICO_ui_line_break(struct RICO_ui_hud *parent);
extern struct RICO_ui_button *RICO_ui_button(struct RICO_ui_hud *parent);
extern struct RICO_ui_label *RICO_ui_label(struct RICO_ui_hud *parent);
extern struct RICO_ui_progress *RICO_ui_progress(struct RICO_ui_hud *parent);
extern bool RICO_ui_layout(struct RICO_ui_element *element, s32 x, s32 y,
                           s32 max_w, s32 max_h);
extern void RICO_ui_draw(struct RICO_ui_element *element, s32 x, s32 y);

#endif