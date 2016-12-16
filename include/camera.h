#ifndef CAMERA_H
#define CAMERA_H

#include "geom.h"
#include "bbox.h"

//------------------------------------------------------------------------------
//TODO: Probably should prefix these?
#define SCREEN_W 1600.0f
#define SCREEN_H 900.0f
// #define SCREEN_W 800.0f
// #define SCREEN_H 600.0f
#define SCREEN_ASPECT SCREEN_W / SCREEN_H

#define Z_NEAR 0.1f
#define Z_FAR 100.0f

struct camera {
    struct vec3 position;
    struct quat view;
    //struct vec3 up;
    float fov_deg;
    GLenum fill_mode;
    bool locked;
    bool need_update;

    struct bbox bbox; //TODO: Replace with vec3 and do vec3_render()
    struct mat4 view_matrix;
    struct mat4 proj_matrix;
};

extern const struct vec3 CAMERA_POS_INITIAL;

void camera_init(struct camera *_camera, struct vec3 position,
                 struct quat view, float fov);
void camera_reset(struct camera *camera);
void camera_translate(struct camera *camera, const struct vec3 *v);
void camera_rotate(struct camera *camera, float mouse_x, float mouse_y);
void camera_rotate_angle(struct camera *camera, struct vec3 axis,
                         float angle_deg);
void camera_update(struct camera *camera);
void camera_render(struct camera *camera);

#endif // CAMERA_H
