#ifndef CAMERA_H
#define CAMERA_H

#include "geom.h"

struct camera {
    struct vec4 scale;
    struct vec4 rot;
    struct vec4 trans;
};

extern struct camera view_camera;
extern struct mat4 view_matrix;
extern GLenum view_polygon_mode;

#endif // CAMERA_H
