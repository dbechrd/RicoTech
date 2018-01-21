struct camera cam_player;

// Note: Height of player's eyes in meters
#define CAMERA_POS_Y_MIN 1.68f
#define CAMERA_FOV_DEG 60.0f
#define QUAT_SCALE_HACK 100.0f

void camera_init(struct camera *camera, struct vec3 position, struct quat view,
                 float fov_deg)
{
    camera->pos = position;
    camera->vel = VEC3_ZERO;
    camera->acc = VEC3_ZERO;
    camera->view = view;
    camera->fov_deg = fov_deg;

    camera->fill_mode = GL_FILL;
    camera->locked = false;
    camera->need_update = true;

    bbox_init(&camera->bbox, VEC3(0.f, 0.f, 0.f), VEC3(1.f, 1.f, 1.f),
              COLOR_WHITE);

    camera->proj_matrix = mat4_init_perspective(SCREEN_W, SCREEN_H, Z_NEAR,
                                                Z_FAR, fov_deg);

    camera_translate_local(camera, &VEC3_ZERO);
    camera_rotate(camera, 0.0f, 0.0f);
    camera_update(camera);
}

void camera_reset(struct camera *camera)
{
    const struct vec3 CAMERA_POS_INITIAL = VEC3(0.0f, CAMERA_POS_Y_MIN, 3.0f);
    camera_init(camera, CAMERA_POS_INITIAL, QUAT_IDENT, CAMERA_FOV_DEG);
}

void camera_translate_world(struct camera *camera, const struct vec3 *v)
{
    v3_add(&camera->pos, v);

    // Prevent camera from going below the floor
    if (camera->pos.y < CAMERA_POS_Y_MIN)
        camera->pos.y = CAMERA_POS_Y_MIN;

    camera->need_update = true;
}

void camera_translate_local(struct camera *camera, const struct vec3 *v)
{
    struct vec3 right = VEC3_RIGHT;
    struct vec3 fwd = VEC3_FWD;

    quat_normalize(&camera->view);
    v3_mul_quat(&right, &camera->view);
    v3_mul_quat(&fwd, &camera->view);

    // Don't slow down when looking down (ignore Y component)
    right.y = EPSILON;
    fwd.y = EPSILON;
    v3_normalize(&right);
    v3_normalize(&fwd);

    camera->pos.x += v->x * right.x + v->z * fwd.x;
    camera->pos.y += v->y;
    camera->pos.z += v->z * fwd.z + v->x * right.z;

    // Prevent camera from going below the floor
    if (camera->pos.y < CAMERA_POS_Y_MIN)
        camera->pos.y = CAMERA_POS_Y_MIN;

    camera->need_update = true;
}

void camera_rotate(struct camera *camera, float dx, float dy)
{
    struct quat pitch;
    quat_from_axis_angle(&pitch, VEC3_X, dy);
    struct quat yaw;
    quat_from_axis_angle(&yaw, VEC3_Y, dx);

    quat_normalize(&pitch);
    quat_normalize(&yaw);

    //"FPS" camera = pitch * view * yaw
    quat_mul(&pitch, &camera->view);
    quat_mul(&pitch, &yaw);
    quat_normalize(&pitch);  // Note: This call might not be necessary?
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

    // HACK: Scale view matrix to decrease quaternion rotation radius. I don't
    //       really understand what this is doing.
    mat4_scalef(&camera->view_matrix, QUAT_SCALE_HACK);

    struct vec3 pos = camera->pos;
    mat4_translate(&camera->view_matrix, v3_negate(&pos));

    // Update audio listener
    alListenerfv(AL_POSITION, (float *)&camera->pos);
    alListenerfv(AL_VELOCITY, (float *)&camera->vel);
    
    struct vec3 fwd = VEC3_FWD;
    v3_mul_quat(&fwd, &camera->view);
    struct vec3 fwd_up[2] = { fwd, VEC3_UP };
    alListenerfv(AL_ORIENTATION, (float *)fwd_up);

    camera->need_update = false;
}

#define CAM_ACC 0.2f
#define CAM_FRICTION_MUL 0.95f
#define LOOK_SENSITIVITY_X 0.1f
#define LOOK_SENSITIVITY_Y 0.1f

void player_camera_update(struct camera *camera, r32 dx, r32 dy,
                          struct vec3 delta_acc)
{
    //---------------------------------------------------
    // Acceleration
    //---------------------------------------------------
    cam_player.acc = delta_acc;

    //---------------------------------------------------
    // Velocity
    //---------------------------------------------------
    // Calculate delta velocity
    // dv' = at;
    struct vec3 acc = camera->acc;
    v3_scalef(&acc, (r32)SIM_SEC);

    // HACK: Other manual adjustments to cam velocity
    // TODO: Adjust acceleration instead?
    v3_scalef(&acc, CAM_ACC / (r32)SIM_SEC);

    v3_add(&camera->vel, &acc);

    // Apply friction (double when slowing down for a more realistic stop)
    float mag_acc = v3_length(&camera->acc);
    if (mag_acc == 0.0f)
    {
        v3_scalef(&camera->vel, 1.0f - (1.0f - CAM_FRICTION_MUL) * 2.0f);
    }
    else
    {
        v3_scalef(&camera->vel, CAM_FRICTION_MUL);
    }

    // Resting check
    float mag_vel = v3_length(&camera->vel);
    if (mag_vel < VEC3_EPSILON)
    {
        camera->vel = VEC3_ZERO;
    }

    //---------------------------------------------------
    // Position
    //---------------------------------------------------
    // Calculate delta position
    // dp' = 1/2at^2 + vt
    struct vec3 vel = camera->acc;
    v3_scalef(&vel, 0.5f * (r32)SIM_SEC * (r32)SIM_SEC);

    struct vec3 vt = camera->vel;
    v3_scalef(&vt, (r32)SIM_SEC);

    v3_add(&vel, &vt);

    // Update position
    camera_translate_local(&cam_player, &vel);

    // TODO: Smooth mouse look somehow
    if (dx != 0 || dy != 0)
    {
        struct vec3 delta;
        delta.x = dx * LOOK_SENSITIVITY_X;
        delta.y = dy * LOOK_SENSITIVITY_Y;
        camera_rotate(&cam_player, delta.x, delta.y);            
    }

    //char buf[128] = { 0 };
    //int len = snprintf(buf, sizeof(buf), "delta_x: %4.f\ndelta_y: %4.f", dx, dy);
    //string_truncate(buf, sizeof(buf), len);
    //string_free_slot(STR_SLOT_DEBUG);
    //load_string(pack_transient, rico_string_slot_string[STR_SLOT_DEBUG],
    //            STR_SLOT_DEBUG, -(FONT_WIDTH * len/2), FONT_HEIGHT,
    //            COLOR_DARK_RED_HIGHLIGHT, 0, NULL, buf);

    camera_update(&cam_player);
}

void camera_render(struct camera *camera)
{
    struct vec3 x = VEC3_RIGHT;
    struct vec3 y = VEC3_UP;
    v3_mul_quat(v3_scalef(&x, 0.1f / QUAT_SCALE_HACK), &camera->view);
    v3_mul_quat(v3_scalef(&y, 0.1f / QUAT_SCALE_HACK), &camera->view);

    struct vec3 x0 = camera->pos;
    struct vec3 x1 = camera->pos;
    struct vec3 y0 = camera->pos;
    struct vec3 y1 = camera->pos;
    v3_sub(&x0, &x);
    v3_add(&x1, &x);
    v3_sub(&y0, &y);
    v3_add(&y1, &y);

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
    v3_mul_quat(&z, &camera->view);
    _ray->orig = camera->pos;
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