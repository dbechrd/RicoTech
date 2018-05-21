#ifndef RICO_BBOX_H
#define RICO_BBOX_H

////////////////////////////////////////////////////////////////////////////////
// TODO: Move this to primitives

// TODO: Implement reuse of data for bounding boxes.. no need to initialize
//       an entirely new vao/vbo for every bbox.
// TODO: Don't serialize vao/vbo!
// TODO: Refactor selected out into linked list of selected objects

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct RICO_bbox
{
    struct vec3 min;
    struct vec3 max;
    bool selected;
};

void RICO_bbox_transform(struct RICO_bbox *bbox, const struct mat4 *m);

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