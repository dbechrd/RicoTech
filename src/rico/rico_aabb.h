#ifndef RICO_BBOX_H
#define RICO_BBOX_H

extern void ric_aabb_transform(struct ric_aabb *aabb, const struct mat4 *m);

static inline bool ric_aabb_intersects(const struct ric_aabb *a,
                                       const struct ric_aabb *b)
{
    return !(a->c.x + a->e.x < b->c.x - b->e.x ||
             b->c.x + b->e.x < a->c.x - a->e.x ||
             a->c.y + a->e.y < b->c.y - b->e.y ||
             b->c.y + b->e.y < a->c.y - a->e.y ||
             a->c.z + a->e.z < b->c.z - b->e.z ||
             b->c.z + b->e.z < a->c.z - a->e.z);
}

#endif