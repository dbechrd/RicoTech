#ifndef RICO_INTERNAL_PRIMITIVES_H
#define RICO_INTERNAL_PRIMITIVES_H

#include "RICO/rico_primitives.h"

extern struct pool_id PRIM_MESH_BBOX;
extern struct pool_id PRIM_MESH_SPHERE;

struct prim_vertex
{
    struct vec3 pos;
    struct vec4 col;
};

static int prim_init();
static void prim_free();

#endif