#ifndef RICO_STATE_H
#define RICO_STATE_H

#define RICO_STATES(f)          \
    f(STATE_ENGINE_INIT)        \
    f(STATE_PLAY_EXPLORE)       \
    f(STATE_EDIT_TRANSLATE)     \
    f(STATE_EDIT_ROTATE)        \
    f(STATE_EDIT_SCALE)         \
    f(STATE_EDIT_TEXTURE)       \
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
internal inline enum rico_state state_get();
internal inline bool is_edit_state(enum rico_state state);
void init_rico_engine();

#endif // RICO_STATE_H
