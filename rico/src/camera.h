#ifndef CAMERA_H
#define CAMERA_H

//TODO: Probably should prefix these? Possibly move to const.h?
#define SCREEN_W 1600
#define SCREEN_H 900
#define SCREEN_ASPECT (float)SCREEN_W / SCREEN_H

#define PIXEL_NORMALIZE_X(x) (((float)x / SCREEN_W * 2.0f))
#define PIXEL_NORMALIZE_Y(y) (((float)y / SCREEN_H * 2.0f))

// Convert pixel coordinates to normalized device coordinates
#define ABS_SCREEN_X(x) (x) / (SCREEN_W / 2.0f) - 1.0f
#define SCREEN_X(x) (x >= 0.0f ? ABS_SCREEN_X(x) : ABS_SCREEN_X(x + SCREEN_W))

#define ABS_SCREEN_Y(y) -(y) / (SCREEN_H / 2.0f) + 1.0f
#define SCREEN_Y(y) (y >= 0.0f ? ABS_SCREEN_Y(y) : ABS_SCREEN_Y(y + SCREEN_H))

#define Z_NEAR 0.01f
#define Z_FAR 1000.0f

struct camera
{
    struct vec3 pos;
    struct vec3 vel;
    struct vec3 acc;

    float pitch;
    float yaw;
    float roll;

    //TODO: Implement better camera with position + lookat. Is that necessary?
    //      Maybe it's easy to derive lookat when I need it? Probably not..
    struct quat view;
    float fov_deg;
    GLenum fill_mode;
    bool locked;
    bool need_update;

    struct bbox bbox; //TODO: Replace with vec3 and do v3_render()
    struct mat4 view_matrix;
    struct mat4 proj_matrix;
};

void camera_init(struct camera *camera, struct vec3 position, struct quat view,
                 float fov_deg);
void camera_reset(struct camera *camera);
void camera_translate_world(struct camera *camera, const struct vec3 *v);
void camera_translate_local(struct camera *camera, const struct vec3 *v);
void camera_rotate(struct camera *camera, float dx, float dy, float dz);
void camera_update(struct camera *camera);
void camera_render(struct camera *camera);
void camera_fwd(struct vec3 *_fwd, struct camera *camera);
void camera_fwd_ray(struct ray *_ray, struct camera *camera);

#endif