#ifndef RICO_STATE_H
#define RICO_STATE_H

#define RICO_STATES(f)          \
    f(STATE_ENGINE_INIT)        \
    f(STATE_PLAY_EXPLORE)       \
    f(STATE_EDIT_TRANSLATE)     \
    f(STATE_EDIT_ROTATE)        \
    f(STATE_EDIT_SCALE)         \
    f(STATE_EDIT_MATERIAL)       \
    f(STATE_EDIT_MESH)          \
    f(STATE_MENU_QUIT)          \
    f(STATE_TEXT_INPUT)         \
    f(STATE_ENGINE_SHUTDOWN)    \
    f(STATE_COUNT)

enum rico_state
{
    RICO_STATES(GEN_LIST)
};
extern const char *rico_state_string[];

int state_update();
extern inline enum rico_state state_get();
extern inline bool state_is_edit();
extern inline bool state_is_paused();
void init_rico_engine();

#endif