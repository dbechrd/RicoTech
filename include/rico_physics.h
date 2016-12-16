#ifndef RICO_PHYSICS_H
#define RICO_PHYSICS_H

#include "geom.h"

struct rico_physics {
    struct vec3 size;
    struct vec3 pos;
    struct vec3 vel;
    struct vec3 acc;
};

struct rico_physics *make_physics(struct vec3 size);
void free_physics(struct rico_physics *);

void update_physics(struct rico_physics *, int count);

#endif