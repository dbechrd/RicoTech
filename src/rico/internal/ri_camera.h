#ifndef RICO_INTERNAL_CAMERA_H
#define RICO_INTERNAL_CAMERA_H

#include "rico_camera.h"

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

static void camera_init(struct RICO_camera *camera, struct vec3 position,
                        struct quat view, float fov_deg);
static void camera_reset(struct RICO_camera *camera);
static void camera_set_fov(struct RICO_camera *camera, float fov_deg);
static void camera_translate_world(struct RICO_camera *camera,
                                   const struct vec3 *v);
static void camera_translate_local(struct RICO_camera *camera,
                                   const struct vec3 *v);
static void camera_rotate(struct RICO_camera *camera, float dx, float dy,
                          float dz);
static void camera_update(struct RICO_camera *camera, r64 sim_alpha);
static void camera_render(struct RICO_camera *camera);
static void camera_fwd_ray(struct ray *_ray, struct RICO_camera *camera);

#endif