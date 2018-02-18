#ifndef RICO_BBOX_H
#define RICO_BBOX_H

////////////////////////////////////////////////////////////////////////////////
// TODO: Move this to primitives

// TODO: Implement reuse of data for bounding boxes.. no need to initialize
//       an entirely new vao/vbo for every bbox.
// TODO: Don't serialize vao/vbo!

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct bbox
{
    struct vec3 min;
    struct vec3 max;
    bool selected;
};

#endif