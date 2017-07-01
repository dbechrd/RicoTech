#ifndef CAMERA_H
#define CAMERA_H

//TODO: Probably should prefix these? Possibly move to const.h?
#define SCREEN_W 1600
#define SCREEN_H 900
#define SCREEN_ASPECT (float)SCREEN_W / SCREEN_H

// Convert pixel coordinates to normalized device coordinates
#define ABS_SCREEN_X(x) (x) / (SCREEN_W / 2.0f) - 1.0f
#define SCREEN_X(x) (x >= 0.0f ? ABS_SCREEN_X(x) : ABS_SCREEN_X(x + SCREEN_W))

#define ABS_SCREEN_Y(y) -(y) / (SCREEN_H / 2.0f) + 1.0f
#define SCREEN_Y(y) (y >= 0.0f ? ABS_SCREEN_Y(y) : ABS_SCREEN_Y(y + SCREEN_H))

#define Z_NEAR 0.01f
#define Z_FAR 200.0f

struct camera {
    struct vec3 position;
    struct quat view;
    //struct vec3 up;
    float fov_deg;
    GLenum fill_mode;
    bool locked;
    bool need_update;

    struct bbox bbox; //TODO: Replace with vec3 and do v3_render()
    struct mat4 view_matrix;
    struct mat4 proj_matrix;
};

#endif // CAMERA_H
