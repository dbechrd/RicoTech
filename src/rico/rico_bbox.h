#ifndef RICO_BBOX_H
#define RICO_BBOX_H

extern void RICO_bbox_transform(struct RICO_bbox *bbox, const struct mat4 *m);

static inline bool RICO_bbox_intersects(const struct RICO_bbox *a,
                                        const struct RICO_bbox *b)
{
    return !(a->max.x < b->min.x ||
             b->max.x < a->min.x ||
             a->max.y < b->min.y ||
             b->max.y < a->min.y ||
             a->max.z < b->min.z ||
             b->max.z < a->min.z);
}

#endif