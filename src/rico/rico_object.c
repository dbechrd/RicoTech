const char *RICO_obj_type_string[] = { RICO_OBJECT_TYPES(GEN_STRING) };

static void object_delete(struct RICO_object *obj)
{
    // TODO: Make sure all of the properties get cleaned up properly
    pack_delete(obj->mesh_id);
    pack_delete(obj->material_id);
}

static struct RICO_object *object_copy(u32 pack, struct RICO_object *other,
                                       const char *name)
{
    pkid new_obj_id = RICO_load_object(pack, other->type, 0, name);
    struct RICO_object *new_obj = RICO_pack_lookup(new_obj_id);

    // TODO: Make sure to update any ref counting we're using to prevent e.g.
    //       mesh or texture from being deleted when still in use.
    void *dst = (u8 *)new_obj + sizeof(struct uid);
    void *src = (u8 *)other + sizeof(struct uid);
    memcpy(dst, src, sizeof(struct RICO_object) - sizeof(struct uid));

    return new_obj;
}
static bool object_selectable(struct RICO_object *obj)
{
    return obj->type != RICO_OBJECT_TYPE_STRING_SCREEN;
}
static void object_select_toggle(struct RICO_object *obj)
{
    obj->selected = !obj->selected;
}
static void object_select(struct RICO_object *obj)
{
    RICO_ASSERT(object_selectable(obj));
    obj->selected = true;
}
static void object_deselect(struct RICO_object *obj)
{
    obj->selected = false;
}
static void object_update_bbox_world(struct RICO_object *obj)
{
    obj->bbox_world = obj->bbox;
    RICO_bbox_transform(&obj->bbox_world, &obj->xform.matrix);
}
static void object_update_obb(struct RICO_object *obj)
{
    struct RICO_obb *obb = &obj->obb;
    obb->e = VEC3(
        (obj->bbox.max.x - obj->bbox.min.x) / 2.0f,
        (obj->bbox.max.y - obj->bbox.min.y) / 2.0f,
        (obj->bbox.max.z - obj->bbox.min.z) / 2.0f
    );
    obb->c = VEC3(
        obj->bbox.min.x + obb->e.x,
        obj->bbox.min.y + obb->e.y,
        obj->bbox.min.z + obb->e.z
    );
    obb->u[0] = VEC3_X;
    obb->u[1] = VEC3_Y;
    obb->u[2] = VEC3_Z;
    // TODO: Calculate third axis using cross product to save memory
    //test_obb.u[2] = v3_cross(&test_obb.u[0], &test_obb.u[1]);

    v3_mul_mat4(&obb->c, &obj->xform.matrix);
    struct quat obb_rot = obj->xform.orientation;
    quat_inverse(&obb_rot);  // TODO: Why do I have to flip this?
    v3_mul_quat(&obb->u[0], &obb_rot);
    v3_mul_quat(&obb->u[1], &obb_rot);
    v3_mul_quat(&obb->u[2], &obb_rot);
    v3_normalize(&obb->u[0]);
    v3_normalize(&obb->u[1]);
    v3_normalize(&obb->u[2]);
    v3_scalef(&obb->u[0], obj->xform.scale.x);
    v3_scalef(&obb->u[1], obj->xform.scale.y);
    v3_scalef(&obb->u[2], obj->xform.scale.z);

    // Make sure we still have a proper box!
    float dot1 = v3_dot(&obb->u[0], &obb->u[1]);
    float dot2 = v3_dot(&obb->u[0], &obb->u[2]);
    float dot3 = v3_dot(&obb->u[1], &obb->u[2]);
    DLB_ASSERT(dot1 == 0.0f);
    DLB_ASSERT(dot2 == 0.0f);
    DLB_ASSERT(dot3 == 0.0f);
}
static void object_update_sphere(struct RICO_object *obj)
{
    struct sphere *sphere = &obj->sphere;
    sphere->orig = obj->obb.c;
    sphere->radius = sqrtf((obj->obb.e.x * obj->obb.e.x +
                            obj->obb.e.y * obj->obb.e.y +
                            obj->obb.e.z * obj->obb.e.z));
}
static void object_update_colliders(struct RICO_object *obj)
{
    if (obj->type == RICO_OBJECT_TYPE_STRING_SCREEN)
        return;

    object_update_bbox_world(obj);
    object_update_obb(obj);
    object_update_sphere(obj);
}
static void object_bbox_recalculate(struct RICO_object *obj)
{
    struct RICO_mesh *mesh;
    if (obj->mesh_id)
    {
        mesh = RICO_pack_lookup(obj->mesh_id);
    }
    else
    {
        mesh = RICO_pack_lookup(MESH_DEFAULT_CUBE);
    }
    RICO_object_bbox_set(obj, &mesh->bbox);
}
static void object_bbox_recalculate_all(u32 id)
{
    struct pack *pack = packs[id];
    struct RICO_object *obj;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type < RICO_OBJECT_TYPE_COUNT)
            continue;

        object_bbox_recalculate(obj);
    }
}
static void object_transform_update(struct RICO_object *obj)
{
    struct RICO_transform *xform = &obj->xform;

    // HACK: Light test
    if (obj->type == OBJ_LIGHT_TEST)
    {
        prog_pbr->frag.lights[0].pos = obj->xform.position;
    }

    // HACK: Order of these operations might not always be the same.. should
    //       probably just store the transformation matrix directly rather than
    //       trying to figure out which order to do what. Unfortunately, doing
    //       this makes relative transformations very difficult. Maybe objects
    //       should have "edit mode" where the matrix is decomposed, then
    //       recomposed again when edit mode is ended?
    struct mat4 transform = MAT4_IDENT;
    mat4_translate(&transform, &xform->position);
    mat4_rot_quat(&transform, &xform->orientation);
    mat4_scale(&transform, &xform->scale);
    xform->matrix = transform;

    struct vec3 scale_inv;
    scale_inv.x = 1.0f / xform->scale.x;
    scale_inv.y = 1.0f / xform->scale.y;
    scale_inv.z = 1.0f / xform->scale.z;

    struct quat orientation_inv = xform->orientation;
    quat_inverse(&orientation_inv);

    struct vec3 position_inv = xform->position;
    v3_negate(&position_inv);

    struct mat4 transform_inverse = MAT4_IDENT;
    mat4_scale(&transform_inverse, &scale_inv);
    mat4_rot_quat(&transform, &orientation_inv);
    mat4_translate(&transform_inverse, &position_inv);
    xform->matrix_inverse = transform_inverse;

    //struct mat4 mm = object->transform;
    //mat4_mul(&mm, &object->transform_inverse);
    //RICO_ASSERT(mat4_equals(&mm, &MAT4_IDENT));

    object_update_colliders(obj);
}
extern void RICO_object_bbox_set(struct RICO_object *obj,
                                 const struct RICO_bbox *bbox)
{
    obj->bbox = *bbox;
    object_update_colliders(obj);
}
extern void RICO_object_mesh_set(struct RICO_object *obj, pkid mesh_id)
{
#if RICO_DEBUG
    struct uid *uid = RICO_pack_lookup(mesh_id);
    RICO_ASSERT(uid->type == RICO_HND_MESH);
#endif
    obj->mesh_id = mesh_id;
    object_bbox_recalculate(obj);
}
extern void RICO_object_material_set(struct RICO_object *obj, pkid material_id)
{
#if RICO_DEBUG
    struct uid *uid = RICO_pack_lookup(material_id);
    RICO_ASSERT(uid->type == RICO_HND_MATERIAL);
#endif
    obj->material_id = material_id;
    object_bbox_recalculate(obj);
}
extern void RICO_object_trans(struct RICO_object *obj, const struct vec3 *v)
{
    v3_add(&obj->xform.position, v);
    object_transform_update(obj);
}
extern const struct vec3 *RICO_object_trans_get(struct RICO_object *obj)
{
    return &obj->xform.position;
}
extern void RICO_object_trans_set(struct RICO_object *obj,
                                  const struct vec3 *v)
{
    obj->xform.position = *v;
    object_transform_update(obj);
}
static void object_rot(struct RICO_object *obj, const struct quat *q)
{
    quat_mul(&obj->xform.orientation, q);
    object_transform_update(obj);
}
static void object_rot_set(struct RICO_object *obj, const struct quat *q)
{
    obj->xform.orientation = *q;
    object_transform_update(obj);
}
static const struct quat *object_rot_get(struct RICO_object *obj)
{
    return &obj->xform.orientation;
}
static void object_scale(struct RICO_object *obj, const struct vec3 *v)
{
    v3_add(&obj->xform.scale, v);
    object_transform_update(obj);
}
static void object_scale_set(struct RICO_object *obj, const struct vec3 *v)
{
    obj->xform.scale = *v;
    object_transform_update(obj);
}
static const struct vec3 *object_scale_get(struct RICO_object *obj)
{
    return &obj->xform.scale;
}
static const struct mat4 *object_matrix_get(struct RICO_object *obj)
{
    return &obj->xform.matrix;
}
static bool object_collide_ray(float *_dist, struct RICO_object *obj,
                               const struct ray *ray)
{
    return collide_ray_obb(_dist, ray, &obj->bbox, &obj->xform.matrix);
}
static bool object_collide_ray_type(pkid *_object_id, float *_dist,
                                    const struct ray *ray)
{
    bool collided = false;
    float closest = Z_FAR; // Track closest object

    struct RICO_object *obj = 0;
    for (u32 i = PACK_COUNT; i < ARRAY_COUNT(packs); ++i)
    {
        if (!packs[i])
            continue;

        for (u32 index = 1; index < packs[i]->blobs_used; ++index)
        {
            if (packs[i]->index[index].type != RICO_HND_OBJECT)
                continue;

            obj = pack_read(packs[i], index);
            if (obj->type == RICO_OBJECT_TYPE_TERRAIN)
                continue;

            float dist;
            bool col = collide_ray_obb(&dist, ray, &obj->bbox,
                                       &obj->xform.matrix);

            // If closest so far, save info
            if (col && dist < closest)
            {
                // Record object handle and distance
                *_object_id = obj->uid.pkid;
                collided = true;
                closest = dist;
            }
        }
    }

    if (_dist) *_dist = closest;
    return collided;
}

static void temp_create_cubemap(GLuint *tex_id, u32 size)
{
    glGenTextures(1, tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *tex_id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Cleanup: Check if these have any effect after shadows are fixed
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Generate the 6 face textures for the cube map
    for (GLint i = 0; i < 6; ++i)
    {
        //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
        //             GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT,
        //             GL_UNSIGNED_INT, NULL);

        // If need more precision (slides):
        //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
        //             GL_DEPTH_COMPONENT32F, w, h, 0, GL_DEPTH_COMPONENT,
        //             GL_FLOAT, NULL);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                     GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT,
                     GL_FLOAT, NULL);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

static void temp_create_framebuffer(GLuint *fbo_id, GLuint tex_id)
{
    glGenFramebuffers(1, fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo_id);

    // Attach shadow cubemap to framebuffer depth
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_id, 0);

    // We don't care about color buffers for shadow map
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#define LIGHT_NEAR 0.01f
#define LIGHT_FAR 50.0f
extern float LIGHT_FOV = 90.0f;
static GLuint shadow_cubemap[NUM_LIGHTS] = { 0 };
static void temp_render_shadow_cubemap(r64 alpha, struct pbr_light *lights)
{
    const u32 tex_size = 1024;

    // TODO: Refactor this debauchery
    static struct mat4 shadow_proj = { 0 };
    static struct mat4 rot_matrices[6] = { 0 };
    static GLuint shadow_fbo[NUM_LIGHTS] = { 0 };

    // Calculate projection matrix
    shadow_proj = mat4_init_perspective(1.0f, LIGHT_NEAR, LIGHT_FAR, LIGHT_FOV);
    //shadow_proj = mat4_init_ortho((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT,
    //                              LIGHT_NEAR, LIGHT_FAR, 0.0f);;

    if (!shadow_cubemap[0])
    {
        //// Calculate projection matrix
        //shadow_proj = mat4_init_perspective((float)tex_w, (float)tex_h,
        //                                    LIGHT_NEAR, LIGHT_FAR, LIGHT_FOV);

        // Calculate rotation matrices
        //rot_matrices[0] = mat4_init_rotx(90.0f);  // pos_x
        //rot_matrices[1] = mat4_init_rotx(-90.0f); // neg_x
        //rot_matrices[2] = mat4_init_roty(90.0f);  // pos_y
        //rot_matrices[3] = mat4_init_roty(-90.0f); // neg_y
        //rot_matrices[4] = mat4_init_rotz(90.0f);  // pos_z
        //rot_matrices[5] = mat4_init_rotz(-90.0f); // neg_z
        rot_matrices[0] = mat4_init(
            0.0f,  0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[1] = mat4_init(
            0.0f,  0.0f,  1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[2] = mat4_init(
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[3] = mat4_init(
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f,  0.0f, -1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[4] = mat4_init(
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
            0.0f,  0.0f, -1.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[5] = mat4_init(
           -1.0f,  0.0f,  0.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  1.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );

        // DEBUG: Print rotation matrices to log file
        for (int i = 0; i < 6; i++)
        {
            printf("rot_matrix[%d]\n", i);
            mat4_print(&rot_matrices[i]);
            fflush(stdout);
        }

        for (int i = 0; i < NUM_LIGHTS; ++i)
        {
            temp_create_cubemap(&shadow_cubemap[i], tex_size);
        }
    }

    if (!shadow_fbo[0])
    {
        for (int i = 0; i < NUM_LIGHTS; ++i)
        {
            temp_create_framebuffer(&shadow_fbo[i], shadow_cubemap[i]);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Render shadows
    glViewport(0, 0, tex_size, tex_size);

    struct shadow_program *prog = prog_shadow;
    glUseProgram(prog->program.gl_id);

    glUniform1f(prog->locations.frag.far_plane, LIGHT_FAR);

    for (int light = 0; light < NUM_LIGHTS; ++light)
    {
        if (!lights[light].enabled)
            continue;

        glUniform3fv(prog->locations.frag.light_pos, 1, &lights[light].pos.x);

        struct mat4 light_views[6] = { 0 };
        struct vec3 light_pos_neg = lights[light].pos;
        v3_negate(&light_pos_neg);

        // projection * rotate * translate
        for (int view = 0; view < 6; ++view)
        {
            light_views[view] = shadow_proj;
            mat4_mul(&light_views[view], &rot_matrices[view]);
            mat4_translate(&light_views[view], &light_pos_neg);

            glUniformMatrix4fv(prog->locations.geom.cubemap_xforms[view], 1,
                               GL_TRUE, light_views[view].a);
        }

        // Clear depth buffer and bind shadow map
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo[light]);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Render scene to this light's shadow map
        for (u32 pack = 1; pack < ARRAY_COUNT(packs); ++pack)
        {
            if (!packs[pack]) continue;
            object_render(packs[pack], prog->locations.vert.model, 0, true);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    ////////////////////////////////////////////////////////////////////////////
}
static void object_render_all(r64 alpha, struct RICO_camera *camera)
{
    // TODO: Interpolate / extrapolate physics
    UNUSED(alpha);

    struct pbr_program *prog = prog_pbr;

    ////////////////////////////////////////////////////////////////////////////
    // Render shadow map
    temp_render_shadow_cubemap(alpha, prog->frag.lights);

    ////////////////////////////////////////////////////////////////////////////
    // Render objects
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RICO_ASSERT(prog->program.gl_id);
    glUseProgram(prog->program.gl_id);

    glUniform2f(prog->locations.vert.scale_uv, 1.0f, 1.0f);

    glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);

    glUniformMatrix4fv(prog->locations.vert.proj, 1, GL_TRUE,
                       camera->proj_matrix->a);
    glUniformMatrix4fv(prog->locations.vert.view, 1, GL_TRUE,
                       camera->view_matrix.a);

    glUniform3fv(prog->locations.frag.camera.pos, 1,
                 (const GLfloat *)&camera->pos);

    // Material textures
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    glUniform1i(prog->locations.frag.material.tex0, 0);
    glUniform1i(prog->locations.frag.material.tex1, 1);
    glUniform1i(prog->locations.frag.material.tex2, 2);

    // Lighting
    for (u32 i = 0; i < NUM_LIGHTS; ++i)
    {
        if (!prog->frag.lights[i].enabled)
            continue;

        glUniform3fv(prog->locations.frag.lights[i].pos, 1,
                     &prog->frag.lights[i].pos.x);
        glUniform3fv(prog->locations.frag.lights[i].color, 1,
                     &prog->frag.lights[i].color.r);
        glUniform1f(prog->locations.frag.lights[i].intensity,
                    prog->frag.lights[i].intensity);
        glUniform1i(prog->locations.frag.lights[i].enabled,
                    prog->frag.lights[i].enabled);

        glActiveTexture(GL_TEXTURE10 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_cubemap[i]);
        glUniform1i(prog->locations.frag.lightmaps[i], 10 + i);
    };

    glUniform1f(prog->locations.frag.far_plane, LIGHT_FAR);

    const GLint model_location = prog->locations.vert.model;
    const GLint scale_uv_location = prog->locations.vert.scale_uv;

    for (u32 i = 1; i < ARRAY_COUNT(packs); ++i)
    {
        if (!packs[i]) continue;
        object_render(packs[i], model_location, scale_uv_location, false);
    }

    glUseProgram(0);

    ////////////////////////////////////////////////////////////////////////////
    // Render bounding boxes
    // TODO: Batch render these
    for (u32 i = 1; i < ARRAY_COUNT(packs); ++i)
    {
        if (!packs[i]) continue;

        for (u32 index = 1; index < packs[i]->blobs_used; ++index)
        {
            if (packs[i]->index[index].type != RICO_HND_OBJECT)
                continue;

            struct RICO_object *obj = pack_read(packs[i], index);
            if (obj->type == RICO_OBJECT_TYPE_STRING_SCREEN)
                continue;

            if (RICO_state_is_edit())
            {
                struct vec4 color = (obj->selected)
                    ? COLOR_RED
                    : COLOR_DARK_WHITE_HIGHLIGHT;
                RICO_prim_draw_bbox_xform(&obj->bbox, &color,
                                          &obj->xform.matrix);
            }
        }
    }
}
static void object_render(struct pack *pack, GLint model_location,
                          GLint scale_uv_location, bool shadow)
{
    struct RICO_object *obj = 0;
    enum RICO_object_type prev_type = RICO_OBJECT_TYPE_NULL;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type == RICO_OBJECT_TYPE_STRING_SCREEN ||
            obj->type == OBJ_LIGHT_TEST)
            continue;

#if RICO_DEBUG_OBJECT
        printf("[ obj][rndr] name=%s\n", object_name(obj));
#endif

        // Set object-specific uniform values

        // Cleanup: This was kinda dumb to begin with. Just set the UV coords
        //          of the terrain properly.
        // UV-coord scale
        // HACK: This only works when object is uniformly scaled on X/Y
        //       plane.
        // TODO: UV scaling in general only works when object is uniformly
        //       scaled. Maybe I should only allow textured objects to be
        //       uniformly scaled?
        if (scale_uv_location)
        {
            if (obj->type == RICO_OBJECT_TYPE_TERRAIN)
		    {
			    glUniform2f(scale_uv_location, 100.0f, 100.0f);
		    }
            else
            {
                glUniform2f(scale_uv_location, obj->xform.scale.x,
                            obj->xform.scale.y);
            }
        }

        glUniformMatrix4fv(model_location, 1, GL_TRUE, obj->xform.matrix.a);

        pkid mat_id = MATERIAL_DEFAULT;
        if (!shadow)
        {
            // Bind material
            if (obj->material_id)
            {
                mat_id = obj->material_id;
            }
            material_bind(mat_id);
        }

        // Render
        pkid mesh_id = MESH_DEFAULT_CUBE;
        if (obj->mesh_id)
        {
            mesh_id = obj->mesh_id;
        }
        glBindVertexArray(mesh_vao(mesh_id));
        mesh_render(mesh_id);
        glBindVertexArray(0);

        // Clean up
        if (!shadow)
        {
            material_unbind(mat_id);
        }
    }
}
static void object_render_ui(struct pack *pack, GLint model_location)
{
    struct RICO_object *obj = 0;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type != RICO_OBJECT_TYPE_STRING_SCREEN)
            continue;

#if RICO_DEBUG_OBJECT
        printf("[ obj][rndr] name=%s\n", object_name(obj));
#endif

        // Model matrix
        //obj->xform.position = VEC3(0.5, 0.5, -1.0f);
        //object_transform_update(obj);
        glUniformMatrix4fv(model_location, 1, GL_TRUE, obj->xform.matrix.a);

        // Bind texture
        pkid tex_id = FONT_DEFAULT_TEXTURE;
        texture_bind(tex_id, GL_TEXTURE0);

        // Render
        pkid mesh_id = 0;
        if (obj->mesh_id)
        {
            mesh_id = obj->mesh_id;
        }
        RICO_ASSERT(mesh_id);
        glBindVertexArray(mesh_vao(mesh_id));
        mesh_render(mesh_id);
        glBindVertexArray(0);

        // Clean up
        texture_unbind(tex_id, GL_TEXTURE0);
    }
}

static void object_print(struct RICO_object *obj)
{
    string_free_slot(STR_SLOT_SELECTED_OBJ);

    int len;
    char buf[BFG_MAXSTRING + 1] = { 0 };

    if (obj)
    {
        struct RICO_mesh *mesh = (obj->mesh_id)
            ? RICO_pack_lookup(obj->mesh_id) : NULL;
        struct RICO_material *material = (obj->material_id)
            ? RICO_pack_lookup(obj->material_id) : NULL;

        const char *mesh_str = "---";
        u32 mesh_verts = 0;
        const char *material_str = "---";
        u32 mat_tex0_id = 0;
        const char *mat_tex0_str = "---";
        u32 mat_tex1_id = 0;
        const char *mat_tex1_str = "---";
        u32 mat_tex2_id = 0;
        const char *mat_tex2_str = "---";

        if (mesh)
        {
            mesh_str = mesh->uid.name;
            mesh_verts = mesh->vertex_count;
        }
        if (material)
        {
            material_str = material->uid.name;

            if (material->tex_albedo)
            {
                struct RICO_texture *tex =
                    RICO_pack_lookup(material->tex_albedo);
                mat_tex0_id = tex->uid.pkid;
                mat_tex0_str = tex->uid.name;
            }
            if (material->tex_mrao)
            {
                struct RICO_texture *tex =
                    RICO_pack_lookup(material->tex_mrao);
                mat_tex1_id = tex->uid.pkid;
                mat_tex1_str = tex->uid.name;
            }
            if (material->tex_emission)
            {
                struct RICO_texture *tex =
                    RICO_pack_lookup(material->tex_emission);
                mat_tex2_id = tex->uid.pkid;
                mat_tex2_str = tex->uid.name;
            }
        }

        len = snprintf(
            buf, sizeof(buf),
            "\n"
            "Object [%u|%u] %s\n"
            "  Type  %d\n"
            "  Pos   %f %f %f\n"
            "  Rot   %f %f %f %f\n"
            "  Scale %f %f %f\n"
            "  AABB  %f %f %f\n"
            "        %f %f %f\n"
            "\n"
            "Mesh [%u|%u] %s\n"
            "  Verts %u\n"
            "\n"
            "Material [%u|%u] %s\n"
            "  Diff [%u|%u] %s\n"
            "  Spec [%u|%u] %s\n"
            "  Emis [%u|%u] %s\n",
            PKID_PACK(obj->uid.pkid), PKID_BLOB(obj->uid.pkid), obj->uid.name,
            obj->type,
            obj->xform.position.x, obj->xform.position.y, obj->xform.position.z,
            obj->xform.orientation.w, obj->xform.orientation.x,
            obj->xform.orientation.y, obj->xform.orientation.z,
            obj->xform.scale.x, obj->xform.scale.y, obj->xform.scale.z,
            obj->bbox_world.min.x, obj->bbox_world.min.y, obj->bbox_world.min.z,
            obj->bbox_world.max.x, obj->bbox_world.max.y, obj->bbox_world.max.z,
            PKID_PACK(obj->mesh_id), PKID_BLOB(obj->mesh_id), mesh_str,
            mesh_verts,
            PKID_PACK(obj->material_id), PKID_BLOB(obj->material_id),
            material_str,
            PKID_PACK(mat_tex0_id), PKID_BLOB(mat_tex0_id), mat_tex0_str,
            PKID_PACK(mat_tex1_id), PKID_BLOB(mat_tex1_id), mat_tex1_str,
            PKID_PACK(mat_tex2_id), PKID_BLOB(mat_tex2_id), mat_tex2_str
        );
    }
    else
    {
        len = snprintf(buf, sizeof(buf), "No object selected");
    }

    string_truncate(buf, sizeof(buf), len);
    RICO_load_string(PACK_TRANSIENT, STR_SLOT_SELECTED_OBJ,
                     SCREEN_X(0), SCREEN_Y(FONT_HEIGHT),
                     COLOR_DARK_WHITE_HIGHLIGHT, 0, 0, buf);
}
