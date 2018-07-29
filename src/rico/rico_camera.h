#ifndef RICO_CAMERA_H
#define RICO_CAMERA_H

extern struct RICO_camera* RICO_get_camera_hack();
extern void RICO_camera_fwd(struct vec3 *_fwd, struct RICO_camera *camera);

#endif