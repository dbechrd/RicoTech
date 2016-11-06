#ifndef CAMERA_H
#define CAMERA_H

#include "geom.h"

//------------------------------------------------------------------------------
//TODO: Probably should prefix these?
#define SCREEN_W 1024
#define SCREEN_H 768

#define Z_NEAR 1.0f
#define Z_FAR 100.0f
#define Z_FOV_DEG 50.0f

struct camera {
    struct vec4 scale;
    struct vec4 rot;
    struct vec4 trans;
};

extern struct camera view_camera;
extern struct mat4 view_matrix;
extern struct mat4 proj_matrix;
extern GLenum view_polygon_mode;

extern bool camera_lock;

#endif // CAMERA_H
