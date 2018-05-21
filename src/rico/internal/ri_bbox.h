#ifndef RICO_INTERNAL_BBOX_H
#define RICO_INTERNAL_BBOX_H

#include "rico_bbox.h"

struct RICO_mesh;

static void bbox_init(struct RICO_bbox *bbox, struct vec3 min, struct vec3 max);
static void bbox_init_mesh(struct RICO_bbox *bbox, struct RICO_mesh *mesh);

#endif