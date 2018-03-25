#ifndef RICO_CAMERA_H
#define RICO_CAMERA_H

struct RICO_camera
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

    struct RICO_bbox RICO_bbox; //TODO: Replace with vec3 and do v3_render()
    struct mat4 view_matrix;
    struct mat4 proj_matrix;
    struct mat4 ortho_matrix;
};

extern struct RICO_camera* RICO_get_camera_hack();
extern void RICO_camera_fwd(struct vec3 *_fwd, struct RICO_camera *camera);

#endif