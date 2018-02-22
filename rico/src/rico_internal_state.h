#ifndef RICO_INTERNAL_STATE_H
#define RICO_INTERNAL_STATE_H

#include "RICO/rico_state.h"

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

enum rico_state { RICO_STATES(GEN_LIST) };
const char *rico_state_string[];

struct pack *pack_active;
float trans_delta;

static inline enum rico_state state_get();
static inline bool state_is_edit();
static inline bool state_is_paused();
static void rico_check_key_events();

#endif
