#ifndef RICO_INTERNAL_STATE_H
#define RICO_INTERNAL_STATE_H

#include "RICO/rico_state.h"

struct pack *pack_active;
float trans_delta;

inline enum rico_state state_get();
inline bool state_is_edit();
inline bool state_is_paused();
void RICO_check_key_events();

#endif