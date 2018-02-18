#ifndef RICO_STATE_H
#define RICO_STATE_H

enum rico_action RICO_key_event();
void RICO_bind_action(enum rico_action action, struct rico_keychord chord);

#endif