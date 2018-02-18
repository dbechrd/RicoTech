#ifndef RICO_INTERNAL_BBOX_H
#define RICO_INTERNAL_BBOX_H

#include "RICO/rico_bbox.h"

struct rico_mesh;

void bbox_init(struct bbox *bbox, struct vec3 min, struct vec3 max);
void bbox_init_mesh(struct bbox *bbox, struct rico_mesh *mesh);

internal inline bool bbox_intersects(const struct bbox *a, const struct bbox *b)
{
    return !(a->max.x < b->min.x ||
             b->max.x < a->min.x ||
             a->max.y < b->min.y ||
             b->max.y < a->min.y ||
             a->max.z < b->min.z ||
             b->max.z < a->min.z);
}

#endif