#ifndef RICO_PHYSICS_H
#define RICO_PHYSICS_H

#include "geom.h"

struct rico_physics {
    struct vec4 size;
    struct vec4 pos;
    struct vec4 vel;
    struct vec4 acc;
};

struct rico_physics *make_physics(struct vec4 size);
void free_physics(struct rico_physics *);

void update_physics(struct rico_physics *, int count);

#endif