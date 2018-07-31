#ifndef RICO_CAMERA_H
#define RICO_CAMERA_H

extern struct ric_camera* ric_get_camera_hack();
extern void ric_camera_fwd(struct ric_camera *camera, struct vec3 *fwd);

#endif