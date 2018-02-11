#ifndef RICO_STATE_H
#define RICO_STATE_H

#define RICO_STATES(f)       \
    f(STATE_PLAY_EXPLORE)    \
    f(STATE_EDIT_TRANSLATE)  \
    f(STATE_EDIT_ROTATE)     \
    f(STATE_EDIT_SCALE)      \
    f(STATE_EDIT_MATERIAL)   \
    f(STATE_EDIT_MESH)       \
    f(STATE_MENU_QUIT)       \
    f(STATE_TEXT_INPUT)      \
    f(STATE_ENGINE_SHUTDOWN) \
    f(STATE_COUNT)

enum rico_state
{
    RICO_STATES(GEN_LIST)
};
extern const char *rico_state_string[];

extern struct pack *pack_active;
extern float trans_delta;

extern inline enum rico_state state_get();
extern inline bool state_is_edit();
extern inline bool state_is_paused();

#endif