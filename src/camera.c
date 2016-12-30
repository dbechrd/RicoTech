#include "camera.h"
#include "bbox.h"
#include "primitives.h"

struct camera cam_player;

//Note: Player's eyes are at 1.7 meters
#define CAMERA_POS_Y_MIN 1.7f
const struct vec3 CAMERA_POS_INITIAL = (struct vec3) {
    0.0f, CAMERA_POS_Y_MIN, 4.0f
};

void camera_init(struct camera *_camera, struct vec3 position,
                 struct quat view, float fov_deg)
{
    _camera->position = position;
    _camera->view = view;
    _camera->fov_deg = fov_deg;

    _camera->fill_mode = GL_FILL;
    _camera->locked = false;
    _camera->need_update = true;

    bbox_init(&_camera->bbox, "camera_bbox",
              (struct vec3) { -0.5f, -0.5f, -0.5f },
              (struct vec3) {  0.5f,  0.5f,  0.5f },
              COLOR_WHITE);

    _camera->proj_matrix = mat4_init_perspective(SCREEN_W, SCREEN_H, Z_NEAR,
                                                 Z_FAR, fov_deg);
}

void camera_reset(struct camera *camera)
{
    struct quat q_view = QUAT_IDENT;

    camera_init(camera,
                CAMERA_POS_INITIAL,
                q_view,
                45.0f);

    camera->need_update = true;
}

void camera_translate(struct camera *camera, const struct vec3 *v)
{
    struct vec3 right = VEC3_RIGHT;
    struct vec3 fwd = VEC3_FWD;

    quat_normalize(&camera->view);
    vec3_mul_quat(&right, &camera->view);
    vec3_mul_quat(&fwd, &camera->view);

    // Don't slow down when looking down (ignore Y component)
    right.y = EPSILON;
    fwd.y = EPSILON;
    vec3_normalize(&right);
    vec3_normalize(&fwd);

    camera->position.x += v->x * right.x + v->z * fwd.x;
    camera->position.y += v->y;
    camera->position.z += v->z * fwd.z + v->x * right.z;

    // Prevent camera from going below the floor
    if (camera->position.y < CAMERA_POS_Y_MIN)
        camera->position.y = CAMERA_POS_Y_MIN;

    camera->need_update = true;
}

void camera_translate_set(struct camera *camera, const struct vec3 *v)
{
    camera->position = VEC3_ZERO;
    camera_translate(camera, v);
}

void camera_rotate(struct camera *camera, float mouse_dx, float mouse_dy)
{
    struct quat pitch;
    quat_from_axis_angle(&pitch, &VEC3_X, mouse_dy * 0.1f);
    struct quat yaw;
    quat_from_axis_angle(&yaw, &VEC3_Y, mouse_dx * 0.1f);

    quat_normalize(&pitch);
    quat_normalize(&yaw);

    //"FPS" camera = pitch * view * yaw
    quat_mul(&pitch, &camera->view);
    quat_mul(&pitch, &yaw);
    quat_normalize(&pitch);
    camera->view = pitch;

    //"Arcball(ish)" camera = yaw * pitch * view
    // quat_mul(&yaw, quat_mul(&pitch, &camera->view));
    // quat_normalize(&yaw);
    // camera->view = yaw;

    camera->need_update = true;
}

void camera_update(struct camera *camera)
{
    if (!camera->need_update)
        return;

    camera->view_matrix = MAT4_IDENT;

    struct mat4 rot;
    mat4_from_quat(&rot, &camera->view);
    mat4_mul(&camera->view_matrix, &rot);

    struct vec3 pos = camera->position;
    mat4_translate(&camera->view_matrix, vec3_negate(&pos));

    camera->need_update = false;
}

void camera_render(struct camera *camera)
{
    struct vec3 x = VEC3_RIGHT;
    struct vec3 y = VEC3_UP;
    vec3_mul_quat(vec3_scalef(&x, 0.1f), &camera->view);
    vec3_mul_quat(vec3_scalef(&y, 0.1f), &camera->view);

    struct vec3 x0 = camera->position;
    struct vec3 x1 = camera->position;
    struct vec3 y0 = camera->position;
    struct vec3 y1 = camera->position;
    vec3_sub(&x0, &x);
    vec3_add(&x1, &x);
    vec3_sub(&y0, &y);
    vec3_add(&y1, &y);

    struct segment x_axis;
    x_axis.vertices[0] = (struct prim_vertex) { x0, COLOR_TRANSPARENT };
    x_axis.vertices[1] = (struct prim_vertex) { x1, COLOR_RED };
    struct segment y_axis;
    y_axis.vertices[0] = (struct prim_vertex) { y0, COLOR_TRANSPARENT };
    y_axis.vertices[1] = (struct prim_vertex) { y1, COLOR_GREEN };

    prim_draw_segment(&x_axis, &MAT4_IDENT, COLOR_WHITE);
    prim_draw_segment(&y_axis, &MAT4_IDENT, COLOR_WHITE);

    ////////////////////////////////////////////////
    // Cleanup: Test code

    // struct mat4 camera_trans = mat4_init_translate(&camera->position);
    // struct mat4 camera_view;
    // mat4_from_quat(&camera_view, &camera->view);
    // mat4_mul(&camera_trans, &camera_view);
    // bbox_render_color(&camera->bbox, camera, &camera_trans, COLOR_BLUE);

    // struct segment s;
    // s.vertices[0].pos = (struct vec3) { 0.0f, 2.0f, 0.0f };
    // s.vertices[0].col = COLOR_WHITE;
    // s.vertices[1].pos = (struct vec3) { 0.0f, 30.0f, 0.0f };
    // s.vertices[1].col = COLOR_MAGENTA;
    // prim_draw_segment(&s, camera, &MAT4_IDENT, COLOR_WHITE);

    // struct ray r;
    // r.pos = (struct vec3) { 0.0f, 30.0f, 0.0f };
    // r.dir = (struct vec3) { 1.0f, 0.0f, 0.0f };
    // prim_draw_ray(&r, camera, &MAT4_IDENT, COLOR_WHITE);
}

void camera_fwd(struct ray *_ray, struct camera *camera)
{
    struct vec3 z = VEC3_FWD;
    vec3_mul_quat(&z, &camera->view);
    _ray->orig = camera->position;
    _ray->dir = z;
}

/*
void camera_rotate_angle(struct camera *camera, struct vec3 axis,
                         float angle_deg)
{
    // TODO: Do I need to normalize this?
    struct quat q_axis;
    quat_from_axis_angle(&q_axis, &axis, angle_deg);
    quat_mul(&camera->view, &q_axis);

    // struct quat qq = q;
    // quat_conjugate(&qq);
    // quat_mul(quat_mul(&qq, &camera->view), &q);

    // camera->view.x = qq->x;
    // camera->view.y = qq->y;
    // camera->view.z = qq->z;
}
*/