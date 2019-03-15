#ifndef RICO_STATE_H
#define RICO_STATE_H

extern r64 ric_simulation_time();
extern bool ric_simulation_paused();
extern void ric_simulation_pause();
extern void ric_simulation_play();
extern bool ric_simulation_prev();
extern enum ric_state ric_state();
extern bool ric_state_is_menu(enum ric_state state);
extern bool ric_state_is_edit(enum ric_state state);
extern int ric_update();
extern void ric_render_objects();
extern void ric_render_editor();
extern void ric_render_crosshair();
extern void ric_render();
extern void ric_frame_swap();
extern bool ric_quit();
extern void ric_mouse_coords(u32 *x, u32 *y);
extern u32 ric_key_event(u32 *action);
extern void ric_bind_action(u32 action, struct ric_keychord chord);
extern struct ric_keychord ric_action_chord(u32 action);
extern const char *ric_key_str(u16 key);
extern void ric_chord_str(struct ric_keychord *chord, char *buf, size_t len);
extern bool ric_mouse_raycast(pkid *_obj_id, float *_dist);

#endif
