#ifndef RICO_INTERNAL_BBOX_H
#define RICO_INTERNAL_BBOX_H

#include "rico_bbox.h"

struct RICO_mesh;

static void bbox_init(struct RICO_bbox *RICO_bbox, struct vec3 min, struct vec3 max);
static void bbox_init_mesh(struct RICO_bbox *RICO_bbox, struct RICO_mesh *mesh);

static inline bool bbox_intersects(const struct RICO_bbox *a, const struct RICO_bbox *b)
{
    return !(a->max.x < b->min.x ||
             b->max.x < a->min.x ||
             a->max.y < b->min.y ||
             b->max.y < a->min.y ||
             a->max.z < b->min.z ||
             b->max.z < a->min.z);
}

#endif