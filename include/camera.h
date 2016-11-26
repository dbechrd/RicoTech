#ifndef CAMERA_H
#define CAMERA_H

#include "geom.h"

//------------------------------------------------------------------------------
//TODO: Probably should prefix these?
#define SCREEN_W 1024
#define SCREEN_H 768
#define SCREEN_ASPECT SCREEN_W / SCREEN_H

#define Z_NEAR 1.0f
#define Z_FAR 100.0f
#define Z_FOV_DEG 50.0f

struct camera {
    struct vec4 scale;
    struct vec4 rot;
    struct vec4 trans;
    struct mat4 view_matrix;
    struct mat4 proj_matrix;
    GLenum fill_mode;
    bool locked;
};

extern struct camera view_camera;

#endif // CAMERA_H
