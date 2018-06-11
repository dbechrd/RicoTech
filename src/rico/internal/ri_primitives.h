#ifndef RICO_INTERNAL_PRIMITIVES_H
#define RICO_INTERNAL_PRIMITIVES_H

#include "rico_primitives.h"

extern struct pool_id PRIM_MESH_BBOX;
extern struct pool_id PRIM_MESH_SPHERE;

static int prim_init();
static void prim_free();
static void prim_draw_line(const struct vec3 *p0, const struct vec3 *p1,
                           const struct vec4 *color, const struct mat4 *xform,
                           const struct mat4 *view, const struct mat4 *proj);
static void prim_draw_quad(u32 vertex_count, const struct prim_vertex *vertices,
                           const struct vec4 *color, const struct mat4 *xform,
                           const struct mat4 *view, const struct mat4 *proj,
                           pkid tex_id);

#endif