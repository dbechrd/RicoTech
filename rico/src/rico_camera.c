struct camera cam_player;

// Note: Height of player's eyes in meters
#define CAMERA_POS_Y_MIN 1.68f
#define CAMERA_FOV_DEG 54.0f
#define QUAT_SCALE_HACK 100.0f

void camera_init(struct camera *camera, struct vec3 position, struct quat view,
                 float fov_deg)
{
    camera->pos = position;
    camera->vel = VEC3_ZERO;
    camera->acc = VEC3_ZERO;
    camera->pitch = 0;
    camera->yaw = 0;
    camera->roll = 0;
    camera->view = view;
    camera->fov_deg = fov_deg;

    camera->fill_mode = GL_FILL;
    camera->locked = false;
    camera->need_update = true;

    bbox_init(&camera->bbox, VEC3(0.f, 0.f, 0.f), VEC3(1.f, 1.f, 1.f));

    camera->proj_matrix = mat4_init_perspective(SCREEN_W, SCREEN_H, Z_NEAR,
                                                Z_FAR, fov_deg);
    camera->ortho_matrix = mat4_init_ortho(SCREEN_W, SCREEN_H, Z_NEAR, Z_FAR);

    camera_translate_local(camera, &VEC3_ZERO);
    camera_rotate(camera, 0.0f, 0.0f, 0.0f);
    camera_update(camera);
}

void camera_reset(struct camera *camera)
{
    const struct vec3 CAMERA_POS_INITIAL = VEC3(0.0f, CAMERA_POS_Y_MIN, 3.0f);
    camera_init(camera, CAMERA_POS_INITIAL, QUAT_IDENT, CAMERA_FOV_DEG);
}

void camera_set_fov(struct camera *camera, float fov_deg)
{
    camera->fov_deg = fov_deg;
    camera->proj_matrix = mat4_init_perspective(SCREEN_W, SCREEN_H, Z_NEAR,
                                                Z_FAR, fov_deg);
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

void camera_rotate(struct camera *camera, float dx, float dy, float dz)
{
    camera->pitch += dx;
    camera->yaw   += dy;
    camera->roll  += dz;

    // Restrain pitch to prevent upside-down camera
    const float pitch_max = 85.0f;
    if (camera->pitch < -pitch_max)
        camera->pitch = -pitch_max;
    else if (camera->pitch > pitch_max)
        camera->pitch = pitch_max;

    // Normalize yaw to unique range [-180.0 <= x < 180.0f]
    if (camera->yaw >= 180.0f) camera->yaw -= 360.0f;
    if (camera->yaw < -180.0f) camera->yaw += 360.0f;

    camera->need_update = true;
}

void camera_update(struct camera *camera)
{
    if (!camera->need_update)
        return;

    struct quat pitch, yaw, roll;
    quat_from_axis_angle(&pitch, VEC3_X, camera->pitch);
    quat_from_axis_angle(&yaw, VEC3_Y, camera->yaw);
    quat_from_axis_angle(&roll, VEC3_Z, camera->roll);

    struct quat orient = pitch;
    quat_mul(&orient, &yaw);
    quat_mul(&orient, &roll);
    quat_normalize(&orient);
    camera->view = orient;

    camera->view_matrix = MAT4_IDENT;

    struct mat4 rot;
    mat4_from_quat(&rot, &orient);
    mat4_mul(&camera->view_matrix, &rot);

    // HACK: Scale view matrix to decrease quaternion rotation radius. I don't
    //       really understand what this is doing.
    mat4_scalef(&camera->view_matrix, QUAT_SCALE_HACK);

    struct vec3 pos = camera->pos;
    mat4_translate(&camera->view_matrix, v3_negate(&pos));

    // Update audio listener
    struct vec3 fwd_up[2] = { VEC3_FWD, VEC3_UP };
    v3_mul_quat(&fwd_up[0], &orient);
    alListenerfv(AL_ORIENTATION, (float *)fwd_up);
    alListenerfv(AL_POSITION, (float *)&camera->pos);
    alListenerfv(AL_VELOCITY, (float *)&camera->vel);

#if RICO_DEBUG_CAMERA
	char buf[128] = { 0 };
	int len = snprintf(buf, sizeof(buf),
					   "    x:%6.1f    \n"
					   "    y:%6.1f    \n"
					   "    z:%6.1f    \n"
					   "pitch:%6.1f deg\n"
                       "  yaw:%6.1f deg\n"
                       " roll:%6.1f deg\n"
					   "  fov:%6.1f deg",
					   camera->pos.x, camera->pos.y, camera->pos.z,
					   camera->pitch, camera->yaw, camera->roll,
                       camera->fov_deg);
	string_truncate(buf, sizeof(buf), len);
	string_free_slot(STR_SLOT_DEBUG_CAMERA);
	RICO_load_string(RICO_packs[PACK_TRANSIENT], STR_SLOT_DEBUG_CAMERA,
                SCREEN_X(-(FONT_WIDTH * 16)), SCREEN_Y(FONT_HEIGHT),
                COLOR_DARK_RED_HIGHLIGHT, 0, NULL, buf);
#endif

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
        camera_rotate(&cam_player, delta.y, delta.x, 0.0f);
    }

    camera_update(&cam_player);
}

void camera_render(struct camera *camera)
{
#if RICO_DEBUG_CAMERA
    struct vec3 x = VEC3_RIGHT;
    struct vec3 y = VEC3_UP;
    v3_mul_quat(v3_scalef(&x, 0.01f / QUAT_SCALE_HACK), &camera->view);
    v3_mul_quat(v3_scalef(&y, 0.01f / QUAT_SCALE_HACK), &camera->view);
    v3_add(&x, &camera->pos);
    v3_add(&y, &camera->pos);

    prim_draw_line(&camera->pos, &x, &MAT4_IDENT, COLOR_RED);
    prim_draw_line(&camera->pos, &y, &MAT4_IDENT, COLOR_GREEN);
#endif
}

void camera_fwd(struct vec3 *_fwd, struct camera *camera)
{
    *_fwd = VEC3_FWD;
    v3_mul_quat(_fwd, &camera->view);
}

void camera_fwd_ray(struct ray *_ray, struct camera *camera)
{
    camera_fwd(&_ray->dir, camera);
    _ray->orig = camera->pos;
}