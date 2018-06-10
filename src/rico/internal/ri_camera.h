#ifndef RICO_INTERNAL_CAMERA_H
#define RICO_INTERNAL_CAMERA_H

#include "rico_camera.h"

//TODO: Probably should prefix these? Possibly move to const.h?
static s32 SCREEN_WIDTH = 1600;
static s32 SCREEN_HEIGHT = 900;
#define SCREEN_ASPECT (float)SCREEN_WIDTH / SCREEN_HEIGHT


#define X_TO_NDC(x) ((float)(x) / (SCREEN_WIDTH / 2.0f) - 1.0f)
#define Y_TO_NDC(y) (-(float)(y) / (SCREEN_HEIGHT / 2.0f) + 1.0f)

// NOTE: Pixel origin is top-left of screen
// NOTE: NDC origin is center of screen
// e.g. [0, 0]     -> [-1.0f, 1.0f]
// e.g. [800, 450] -> [0.0f, 0.0f]

// Calculate relative x/y in pixels, returns x/y in normalized device
// coordinates (negative values are relative to right and bottom edges of
// screen)
#define SCREEN_X(x) (x >= 0.0f ? X_TO_NDC(x) : X_TO_NDC(x + SCREEN_WIDTH))
#define SCREEN_Y(y) (y >= 0.0f ? Y_TO_NDC(y) : Y_TO_NDC(y + SCREEN_HEIGHT))

// Takes width/height in pixels, returns width/height in NDC
#define SCREEN_W(x) ((float)(x) / SCREEN_WIDTH * 2.0f)
#define SCREEN_H(y) (-(float)(y) / SCREEN_HEIGHT * 2.0f)

#define Z_NEAR 0.01f
#define Z_FAR 1000.0f

static void camera_init(struct RICO_camera *camera, struct vec3 position,
                        struct quat view, float fov_deg);
static void camera_reset(struct RICO_camera *camera);
static void camera_set_fov(struct RICO_camera *camera, float fov_deg);
static void camera_toggle_projection(struct RICO_camera *camera);
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