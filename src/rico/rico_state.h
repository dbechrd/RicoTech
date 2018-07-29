#ifndef RICO_STATE_H
#define RICO_STATE_H

extern r64 RICO_simulation_time();
extern bool RICO_simulation_paused();
extern void RICO_simulation_pause();
extern void RICO_simulation_play();
extern bool RICO_simulation_prev();
extern enum ric_state RICO_state();
extern bool RICO_state_is_menu();
extern bool RICO_state_is_edit();
extern int RICO_update();
extern void RICO_render_objects();
extern void RICO_render_editor();
extern void RICO_render_crosshair();
extern void RICO_render();
extern void RICO_frame_swap();
extern bool RICO_quit();
extern void RICO_mouse_coords(u32 *x, u32 *y);
extern u32 RICO_key_event(u32 *action);
extern void RICO_bind_action(u32 action, struct RICO_keychord chord);
extern bool RICO_mouse_raycast(pkid *_obj_id, float *_dist);

#endif
