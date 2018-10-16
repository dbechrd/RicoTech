static void object_delete(struct ric_object *obj)
{
    // TODO: Make sure all of the properties get cleaned up properly
    pack_delete(obj->mesh_id);
    pack_delete(obj->material_id);
}

static struct ric_object *object_copy(u32 pack, struct ric_object *other,
                                       const char *name)
{
    pkid new_obj_id = ric_load_object(pack, other->type, 0, name);
    struct ric_object *new_obj = ric_pack_lookup(new_obj_id);

    // TODO: Make sure to update any ref counting we're using to prevent e.g.
    //       mesh or texture from being deleted when still in use.
    void *dst = (u8 *)new_obj + sizeof(struct ric_uid);
    void *src = (u8 *)other + sizeof(struct ric_uid);
    memcpy(dst, src, sizeof(struct ric_object) - sizeof(struct ric_uid));

    return new_obj;
}
static void object_update_obb(struct ric_object *obj)
{
    struct ric_obb *obb = &obj->obb;
    obb->e = obj->aabb.e;
    obb->c = obj->aabb.c;
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

#if RICO_DEBUG
    // Cleanup: Make sure we still have a proper box!
    float dot1 = v3_dot(&obb->u[0], &obb->u[1]);
    float dot2 = v3_dot(&obb->u[0], &obb->u[2]);
    float dot3 = v3_dot(&obb->u[1], &obb->u[2]);
    DLB_ASSERT(dot1 == 0.0f);
    DLB_ASSERT(dot2 == 0.0f);
    DLB_ASSERT(dot3 == 0.0f);
#endif
}
static void object_update_aabb_world(struct ric_object *obj)
{
    obj->aabb_world.c = obj->obb.c;
    obj->aabb_world.e = VEC3_ZERO;

    for (int i = 0; i < 3; ++i)
    {
        struct vec3 axis = obj->obb.u[i];
        v3_scalef(&axis, obj->obb.e.a[i]);

        obj->aabb_world.e.x += ABS(v3_dot(&axis, &VEC3_X));
        obj->aabb_world.e.y += ABS(v3_dot(&axis, &VEC3_Y));
        obj->aabb_world.e.z += ABS(v3_dot(&axis, &VEC3_Z));
    }
}
static void object_update_sphere(struct ric_object *obj)
{
    struct sphere *sphere = &obj->sphere;
    sphere->center = obj->obb.c;
    sphere->r = sqrtf((obj->obb.e.x * obj->obb.e.x +
                            obj->obb.e.y * obj->obb.e.y +
                            obj->obb.e.z * obj->obb.e.z));
}
static void object_update_colliders(struct ric_object *obj)
{
    object_update_obb(obj);
    object_update_aabb_world(obj);
    object_update_sphere(obj);
}
static void object_aabb_recalculate(struct ric_object *obj)
{
    struct ric_mesh *mesh;
    if (obj->mesh_id)
    {
        mesh = ric_pack_lookup(obj->mesh_id);
    }
    else
    {
        mesh = ric_pack_lookup(global_default_mesh_cube);
    }
    ric_object_aabb_set(obj, &mesh->aabb);
}
static void object_aabb_recalculate_all(u32 id)
{
    struct pack *pack = global_packs[id];
    struct ric_object *obj;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RIC_ASSET_OBJECT)
            continue;

        obj = pack_read(pack, index);
        object_aabb_recalculate(obj);
    }
}
static void object_transform_update(struct ric_object *obj)
{
    struct ric_transform *xform = &obj->xform;

    // HACK: Light test
    if (obj->type == OBJ_LIGHT_TEST)
    {
        DLB_ASSERT(global_prog_pbr->val.frag.lights[NUM_LIGHT_DIR].type == RIC_LIGHT_POINT);
        global_prog_pbr->val.frag.lights[NUM_LIGHT_DIR].position = obj->xform.position;
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
extern void ric_object_aabb_set(struct ric_object *obj,
                                 const struct ric_aabb *aabb)
{
    obj->aabb = *aabb;
    object_update_colliders(obj);
}
extern void ric_object_mesh_set(struct ric_object *obj, pkid mesh_id)
{
#if RICO_DEBUG
    struct ric_uid *uid = ric_pack_lookup(mesh_id);
    RICO_ASSERT(uid->type == RIC_ASSET_MESH);
#endif
    obj->mesh_id = mesh_id;
    object_aabb_recalculate(obj);
}
extern void ric_object_material_set(struct ric_object *obj, pkid material_id)
{
#if RICO_DEBUG
    struct ric_uid *uid = ric_pack_lookup(material_id);
    RICO_ASSERT(uid->type == RIC_ASSET_MATERIAL);
#endif
    obj->material_id = material_id;
}
extern void ric_object_trans(struct ric_object *obj, const struct vec3 *v)
{
    v3_add(&obj->xform.position, v);
    object_transform_update(obj);
}
extern const struct vec3 *ric_object_trans_get(struct ric_object *obj)
{
    return &obj->xform.position;
}
extern void ric_object_trans_set(struct ric_object *obj,
                                  const struct vec3 *v)
{
    obj->xform.position = *v;
    object_transform_update(obj);
}
static void object_rot(struct ric_object *obj, const struct quat *q)
{
    quat_mul(&obj->xform.orientation, q);
    object_transform_update(obj);
}
static void object_rot_set(struct ric_object *obj, const struct quat *q)
{
    obj->xform.orientation = *q;
    object_transform_update(obj);
}
static const struct quat *object_rot_get(struct ric_object *obj)
{
    return &obj->xform.orientation;
}
static void object_scale(struct ric_object *obj, const struct vec3 *v)
{
    v3_add(&obj->xform.scale, v);
    object_transform_update(obj);
}
static void object_scale_set(struct ric_object *obj, const struct vec3 *v)
{
    obj->xform.scale = *v;
    object_transform_update(obj);
}
static const struct vec3 *object_scale_get(struct ric_object *obj)
{
    return &obj->xform.scale;
}
static const struct mat4 *object_matrix_get(struct ric_object *obj)
{
    return &obj->xform.matrix;
}
static bool object_collide_ray(float *_dist, struct ric_object *obj,
                               const struct ray *ray)
{
    return collide_ray_obb(_dist, ray, &obj->aabb, &obj->xform.matrix);
}
static bool object_collide_ray_type(pkid *_object_id, float *_dist,
                                    const struct ray *ray)
{
    bool collided = false;
    float closest = Z_FAR; // Track closest object

    struct ric_object *obj = 0;
    for (u32 i = RIC_PACK_ID_COUNT; i < ARRAY_COUNT(global_packs); ++i)
    {
        if (!global_packs[i])
            continue;

        for (u32 index = 1; index < global_packs[i]->blobs_used; ++index)
        {
            if (global_packs[i]->index[index].type != RIC_ASSET_OBJECT)
                continue;

            obj = pack_read(global_packs[i], index);
            if (obj->select_ignore)
                continue;

            float dist;
            //bool col = collide_ray_obb(&dist, ray, &obj->aabb,
            //                           &obj->xform.matrix);
            bool col = collide_ray_sphere(&dist, ray, &obj->sphere);

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

static void object_render_all(r64 alpha, struct ric_camera *camera)
{
    // TODO: Interpolate / extrapolate physics
    UNUSED(alpha);

    struct program_pbr *prog = global_prog_pbr;

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RICO_ASSERT(prog->program.gl_id);
    glUseProgram(prog->program.gl_id);

    r64 time = ric_simulation_time();
    glUniform1f(prog->locations.frag.time, (r32)SIM_SEC);  // Cleanup: Shader time

    time /= 2000.0;
    float x = (float)cos(time);
    float y = (float)-ABS(sin(time));
    struct vec3 new_dir = VEC3(x, y, 0.0f);
    v3_normalize(&new_dir);
    //printf("Light: %f, %f\n", new_dir.x, new_dir.y);
    prog->val.frag.lights[0].directional.direction = new_dir;

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
    for (u32 i = 0; i < NUM_LIGHT_DIR + NUM_LIGHT_POINT; ++i)
    {
        if (!prog->val.frag.lights[i].on)
            continue;

        glUniform1i(prog->locations.frag.lights[i].type,
                    prog->val.frag.lights[i].type);
        glUniform1i(prog->locations.frag.lights[i].on,
                    prog->val.frag.lights[i].on);
        glUniform3fv(prog->locations.frag.lights[i].col, 1,
                     &prog->val.frag.lights[i].color.r);
        glUniform3fv(prog->locations.frag.lights[i].pos, 1,
                     &prog->val.frag.lights[i].position.x);
        glUniform1f(prog->locations.frag.lights[i].intensity,
                    prog->val.frag.lights[i].intensity);
        glUniform3fv(prog->locations.frag.lights[i].dir, 1,
                     &prog->val.frag.lights[i].directional.direction.x);

        // TODO: Move this somewhere sane
#       define SHADOW_TEXTURE_OFFSET 4
        glActiveTexture(GL_TEXTURE0 + SHADOW_TEXTURE_OFFSET + i);
        if (prog->val.frag.lights[i].type == RIC_LIGHT_DIRECTIONAL)
        {
            glBindTexture(GL_TEXTURE_2D, shadow_textures[i]);
            glUniform1i(prog->locations.frag.shadow_textures[i],
                        SHADOW_TEXTURE_OFFSET + i);

            // TODO: Make vert.light_space a proper array
            glUniformMatrix4fv(prog->locations.vert.light_space, 1, GL_TRUE,
                               shadow_lightspace[0].a);
        }
        else if (prog->val.frag.lights[i].type == RIC_LIGHT_POINT)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_cubemaps[i - NUM_LIGHT_DIR]);
            glUniform1i(prog->locations.frag.shadow_cubemaps[i- NUM_LIGHT_DIR],
                        SHADOW_TEXTURE_OFFSET + i);
        }
    };

    glUniform2f(prog->locations.frag.near_far, LIGHT_NEAR, LIGHT_FAR);

    const GLint model_location = prog->locations.vert.model;

    for (u32 i = 1; i < ARRAY_COUNT(global_packs); ++i)
    {
        if (!global_packs[i]) continue;
        object_render(global_packs[i], model_location, false);
    }

    glUseProgram(0);

    ////////////////////////////////////////////////////////////////////////////
    // Render bounding boxes
    // TODO: Batch render these
    for (u32 i = 1; i < ARRAY_COUNT(global_packs); ++i)
    {
        if (!global_packs[i]) continue;

        for (u32 index = 1; index < global_packs[i]->blobs_used; ++index)
        {
            if (global_packs[i]->index[index].type != RIC_ASSET_OBJECT)
                continue;

            struct ric_object *obj = pack_read(global_packs[i], index);

            if (ric_state_is_edit())
            {
                struct vec4 color = (obj->selected)
                    ? COLOR_RED
                    : COLOR_DARK_WHITE_HIGHLIGHT;
                ric_prim_draw_aabb_xform(&obj->aabb, &color,
                                          &obj->xform.matrix);
            }
        }
    }
}
static void object_render(struct pack *pack, GLint model_location, bool shadow)
{
    struct ric_object *obj = 0;
    u32 prev_type = 0;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RIC_ASSET_OBJECT)
            continue;

        obj = pack_read(pack, index);

#if RICO_DEBUG_OBJECT
        printf("[ obj][rndr] name=%s\n", object_name(obj));
#endif

        // Set object-specific uniform values
        glUniformMatrix4fv(model_location, 1, GL_TRUE, obj->xform.matrix.a);

        pkid mat_id = global_default_material;
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
        pkid mesh_id = global_default_mesh_cube;
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

static void object_print(struct ric_object *obj)
{
    string_free_slot(RIC_STRING_SLOT_SELECTED_OBJ);

    int len;
    char buf[BFG_MAXSTRING + 1] = { 0 };

    if (obj)
    {
        DLB_ASSERT(obj->uid.type == RIC_ASSET_OBJECT);
        buf32 *pack_name = &global_packs[PKID_PACK(obj->uid.pkid)]->name;

        struct ric_mesh *mesh = (obj->mesh_id)
            ? ric_pack_lookup(obj->mesh_id) : NULL;
        struct ric_material *material = (obj->material_id)
            ? ric_pack_lookup(obj->material_id) : NULL;

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
                struct ric_texture *tex =
                    ric_pack_lookup(material->tex_albedo);
                mat_tex0_id = tex->uid.pkid;
                mat_tex0_str = tex->uid.name;
            }
            if (material->tex_mrao)
            {
                struct ric_texture *tex =
                    ric_pack_lookup(material->tex_mrao);
                mat_tex1_id = tex->uid.pkid;
                mat_tex1_str = tex->uid.name;
            }
            if (material->tex_emission)
            {
                struct ric_texture *tex =
                    ric_pack_lookup(material->tex_emission);
                mat_tex2_id = tex->uid.pkid;
                mat_tex2_str = tex->uid.name;
            }
        }

        len = snprintf(
            buf, sizeof(buf),
            "\n"
            "Object\n"
            "  Pack  %u: %s\n"
            "  Blob  %u: %s\n"
            "  Type  %d\n"
            "  Pos   %f %f %f\n"
            "  Rot   %f %f %f %f\n"
            "  Scale %f %f %f\n"
            "  AABB   center: %f %f %f\n"
            "        extents: %f %f %f\n"
            "\n"
            "Mesh [%u|%u] %s\n"
            "  Verts %u\n"
            "\n"
            "Material [%u|%u] %s\n"
            "  Diff [%u|%u] %s\n"
            "  Spec [%u|%u] %s\n"
            "  Emis [%u|%u] %s\n",
            PKID_PACK(obj->uid.pkid), pack_name,
            PKID_BLOB(obj->uid.pkid), obj->uid.name,
            obj->type,
            obj->xform.position.x, obj->xform.position.y, obj->xform.position.z,
            obj->xform.orientation.w, obj->xform.orientation.x,
            obj->xform.orientation.y, obj->xform.orientation.z,
            obj->xform.scale.x, obj->xform.scale.y, obj->xform.scale.z,
            obj->aabb_world.c.x, obj->aabb_world.c.y, obj->aabb_world.c.z,
            obj->aabb_world.e.x, obj->aabb_world.e.y, obj->aabb_world.e.z,
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
    ric_load_string(RIC_PACK_ID_TRANSIENT, RIC_STRING_SLOT_SELECTED_OBJ,
                     SCREEN_X(0), SCREEN_Y(FONT_HEIGHT),
                     COLOR_DARK_WHITE_HIGHLIGHT, 0, 0, buf);
}
