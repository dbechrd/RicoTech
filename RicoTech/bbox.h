#ifndef BBOX_H
#define BBOX_H

#include "geom.h"
//#include "program.h"
//#include <GL/gl3w.h>
//#include <malloc.h>
//#include <math.h>
//#include <stdio.h>
//#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////

#ifndef BBOX_EPSILON
#define BBOX_EPSILON 0.1f
#endif

struct bbox {
    GLuint vao;
    GLuint vbos[2];
    struct program_bbox *program;
    struct vec4 vertices[8];
    //struct vec4 p0;
    //struct vec4 p1;

    //TODO: Make BBox shader program that takes color as uniform?
    struct col4 color;
};

void bbox_init(struct bbox *box);
void bbox_render(const struct bbox *box, mat4 model_matrix);
void bbox_render_color(const struct bbox *box, mat4 model_matrix,
                       const struct col4 color);

static inline struct bbox *make_bbox(struct vec4 p0, struct vec4 p1,
                                     struct col4 color)
{
    struct bbox *bbox = (struct bbox *)calloc(1, sizeof(struct bbox));
    bbox->program = make_program_bbox();
    bbox->vertices[0] = (struct vec4) { p0.x, p0.y, p0.z, 1.0f };
    bbox->vertices[1] = (struct vec4) { p1.x, p0.y, p0.z, 1.0f };
    bbox->vertices[2] = (struct vec4) { p1.x, p1.y, p0.z, 1.0f };
    bbox->vertices[3] = (struct vec4) { p0.x, p1.y, p0.z, 1.0f };
    bbox->vertices[4] = (struct vec4) { p0.x, p0.y, p1.z, 1.0f };
    bbox->vertices[5] = (struct vec4) { p1.x, p0.y, p1.z, 1.0f };
    bbox->vertices[6] = (struct vec4) { p1.x, p1.y, p1.z, 1.0f };
    bbox->vertices[7] = (struct vec4) { p0.x, p1.y, p1.z, 1.0f };
    bbox->color = color;
    return bbox;
}

static inline struct bbox *make_bbox_mesh(const struct vertex *verts,
                                          int count,
                                          struct col4 color)
{
    struct vec4 p0 = (struct vec4) { 9999.0f, 9999.0f, 9999.0f };
    struct vec4 p1 = (struct vec4) { -9999.0f, -9999.0f, -9999.0f };

    // Find bounds of mesh
    for (int i = 0; i < count; ++i)
    {
        if (verts[i].pos.x < p0.x)
            p0.x = verts[i].pos.x;
        else if (verts[i].pos.x > p1.x)
            p1.x = verts[i].pos.x;

        if (verts[i].pos.y < p0.y)
            p0.y = verts[i].pos.y;
        else if (verts[i].pos.y > p1.y)
            p1.y = verts[i].pos.y;

        if (verts[i].pos.z < p0.z)
            p0.z = verts[i].pos.z;
        else if (verts[i].pos.z > p1.z)
            p1.z = verts[i].pos.z;
    }

    // Prevent infinitesimally small bounds
    if (p0.x == p1.x)
    {
        p0.x -= BBOX_EPSILON;
        p1.x += BBOX_EPSILON;
    }
    if (p0.y == p1.y)
    {
        p0.y -= BBOX_EPSILON;
        p1.y += BBOX_EPSILON;
    }
    if (p0.z == p1.z)
    {
        p0.z -= BBOX_EPSILON;
        p1.z += BBOX_EPSILON;
    }

    return make_bbox(p0, p1, color);
}

static inline bool bbox_intersects(const struct bbox *a, const struct bbox *b)
{
    if (a->vertices[7].x < b->vertices[0].x) return false;
    if (b->vertices[7].x < a->vertices[0].x) return false;

    if (a->vertices[7].y < b->vertices[0].y) return false;
    if (b->vertices[7].y < a->vertices[0].y) return false;

    if (a->vertices[7].z < b->vertices[0].z) return false;
    if (b->vertices[7].z < a->vertices[0].z) return false;

    return true;
}

static inline void free_bbox(struct bbox **bbox)
{
    free(*bbox);
    *bbox = NULL;
}

#endif // BBOX_H
