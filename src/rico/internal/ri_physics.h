#ifndef RICO_INTERNAL_PHYSICS_H
#define RICO_INTERNAL_PHYSICS_H

//#include "geom.h"

struct rico_physics
{
    struct vec3 size;
    struct vec3 pos;
    struct vec3 vel;
    struct vec3 acc;
};

static struct rico_physics *make_physics(struct vec3 size);
static void free_physics(struct rico_physics *phys);
static void update_physics(struct rico_physics *phys, int bucket_count);

#endif