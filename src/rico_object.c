const char *rico_obj_type_string[] = {
    RICO_OBJ_TYPES(GEN_STRING)
};

struct rico_object *RICO_DEFAULT_OBJECT;

internal void update_transform(struct rico_object *obj);

int object_init(struct rico_object *object, const char *name,
                enum rico_obj_type type, struct rico_mesh *mesh,
                struct rico_material *material, const struct bbox *bbox)
{
    enum rico_error err;

#if RICO_DEBUG_OBJECT
    printf("[ obj][init] name=%s\n", name);
#endif

    hnd_init(&object->hnd, RICO_HND_OBJECT, name);
    object->type = type;
    object->trans = VEC3_ZERO;
    object->rot = VEC3_ZERO;
    if (type == OBJ_STRING_SCREEN)
        object->scale = VEC3_SCALE_ASPECT;
    else
        object->scale = VEC3_ONE;
    object->transform = MAT4_IDENT;
    object->transform_inverse = MAT4_IDENT;
    object->mesh = mesh;
    mesh->ref_count++;
    object->material = material;
    material->ref_count++;
    object->bbox = (bbox != NULL) ? *bbox : object->mesh->bbox;

    update_transform(object);

    return err;
}

int object_copy(struct rico_object *object, struct rico_object *other,
                const char *name)
{
    enum rico_error err;

    // Create new object with same mesh / texture
    err = object_init(object, name, other->type, other->mesh, other->material,
                      NULL);
    if (err) return err;

    // TODO: Make transform one property and add optional param to object_init
    // Copy transform
    object_trans_set(object, &other->trans);
    object_rot_set(object, &other->rot);
    object_scale_set(object, &other->scale);

    return err;
}

void object_free(struct rico_object *object)
{
    // Cleanup: Use fixed pool slots
    //if (handle.value == RICO_DEFAULT_OBJECT)
    //    return;

#if RICO_DEBUG_OBJECT
    printf("[ obj][free] uid=%d name=%s\n", object->hnd.uid, object->hnd.name);
#endif

    mesh_free(object->mesh);
    material_free(object->material);

    object->hnd.uid = UID_NULL;
    struct rico_pool *pool = chunk_pool(chunk_active, RICO_HND_OBJECT);
    pool_handle_free(pool, &object->hnd);
}

void object_free_all()
{
    struct rico_pool *pool = chunk_pool(chunk_active, RICO_HND_OBJECT);
    for (int i = pool->active - 1; i >= 0; --i)
    {
        object_free((struct rico_object *)pool->handles[i]);
    }
}

void object_bbox_recalculate_all()
{
    struct rico_pool *pool = chunk_pool(chunk_active, RICO_HND_OBJECT);
    for (u32 i = 0; i < pool->active; ++i)
    {
        object_bbox_set((struct rico_object *)pool->handles[i], NULL);
    }
}

void object_bbox_set(struct rico_object *object,
                     const struct bbox *bbox)
{
    object->bbox = (bbox != NULL) ? *bbox : object->mesh->bbox;
}

void object_mesh_set(struct rico_object *object, struct rico_mesh *mesh,
                     const struct bbox *bbox)
{
    mesh_free(object->mesh);
    object->mesh = mesh;
    mesh->ref_count++;
    object->bbox = (bbox != NULL) ? *bbox : object->mesh->bbox;
}

void object_mesh_next(struct rico_object *object)
{
    struct rico_mesh *next_mesh = mesh_next(object->mesh);
    if (next_mesh->hnd.uid == object->mesh->hnd.uid)
        return;

    mesh_free(object->mesh);
    object->mesh = next_mesh;
    next_mesh->ref_count++;
    object->bbox = next_mesh->bbox;
}

void object_mesh_prev(struct rico_object *object)
{
    struct rico_mesh *prev_mesh = mesh_prev(object->mesh);
    if (prev_mesh->hnd.uid == object->mesh->hnd.uid)
        return;

    mesh_free(object->mesh);
    object->mesh = prev_mesh;
    prev_mesh->ref_count++;
    object->bbox = prev_mesh->bbox;
}

void object_material_set(struct rico_object *object, struct rico_material *material)
{
    material_free(object->material);
    object->material = material;
    material->ref_count++;
}

bool object_selectable(struct rico_object *object)
{
    return (object->type != OBJ_STRING_SCREEN);
}

// TODO: If these _next / _prev functions actually need to exist, we should be
//       be passing the pool as well, or it should be embedded in the object's
//       handle.
struct rico_object *object_next(struct rico_object *object)
{
    struct rico_pool *pool = chunk_pool(chunk_active, RICO_HND_OBJECT);
    struct hnd *start = pool_handle_next(pool, &object->hnd);
    struct hnd *next = start;

    struct rico_object *obj;
    do
    {
        obj = (struct rico_object *)next;
        if (object_selectable(obj))
            return obj;

        next = pool_handle_next(pool, next);
    } while (next->uid != start->uid);

    return NULL;
}

struct rico_object *object_prev(struct rico_object *object)
{
    struct rico_pool *pool = chunk_pool(chunk_active, RICO_HND_OBJECT);
    struct hnd *start = pool_handle_prev(pool, &object->hnd);
    struct hnd *prev = start;

    struct rico_object *obj;
    do
    {
        obj = (struct rico_object *)prev;
        if (object_selectable(obj))
            return obj;

        prev = pool_handle_prev(pool, prev);
    } while (prev->uid != start->uid);

    return NULL;
}

void object_select(struct rico_object *object)
{
    object->bbox.wireframe = false;
}

void object_deselect(struct rico_object *object)
{
    object->bbox.wireframe = true;
}

void object_trans(struct rico_object *object, const struct vec3 *v)
{
    v3_add(&object->trans, v);
    update_transform(object);
}

const struct vec3 *object_trans_get(struct rico_object *object)
{
    return &object->trans;
}

void object_trans_set(struct rico_object *object,
                      const struct vec3 *v)
{
    object->trans = *v;
    update_transform(object);
}

void object_rot(struct rico_object *object, const struct vec3 *v)
{
    v3_add(&object->rot, v);
    update_transform(object);
}

void object_rot_set(struct rico_object *object, const struct vec3 *v)
{
    object->rot = *v;
    update_transform(object);
}

const struct vec3 *object_rot_get(struct rico_object *object)
{
    return &object->rot;
}

void object_rot_x(struct rico_object *object, float deg)
{
    object->rot.x += deg;
    update_transform(object);
}

void object_rot_x_set(struct rico_object *object, float deg)
{
    object->rot.x = deg;
    update_transform(object);
}

void object_rot_y(struct rico_object *object, float deg)
{
    object->rot.y += deg;
    update_transform(object);
}

void object_rot_y_set(struct rico_object *object, float deg)
{
    object->rot.y = deg;
    update_transform(object);
}

void object_rot_z(struct rico_object *object, float deg)
{
    object->rot.z += deg;
    update_transform(object);
}

void object_rot_z_set(struct rico_object *object, float deg)
{
    object->rot.z = deg;
    update_transform(object);
}

void object_scale(struct rico_object *object, const struct vec3 *v)
{
    v3_add(&object->scale, v);
    update_transform(object);
}

void object_scale_set(struct rico_object *object,
                      const struct vec3 *v)
{
    object->scale = *v;
    update_transform(object);
}

const struct vec3 *object_scale_get(struct rico_object *object)
{
    return &object->scale;
}

internal void update_transform(struct rico_object *object)
{
    //HACK: Order of these operations might not always be the same.. should
    //      probably just store the transformation matrix directly rather than
    //      trying to figure out which order to do what. Unfortunately, doing
    //      this makes relative transformations very difficult. Maybe objects
    //      should have "edit mode" where the matrix is decomposed, then
    //      recomposed again when edit mode is ended?
    object->transform = MAT4_IDENT;
    mat4_translate(&object->transform, &object->trans);
    mat4_rotx(&object->transform, object->rot.x);
    mat4_roty(&object->transform, object->rot.y);
    mat4_rotz(&object->transform, object->rot.z);
    mat4_scale(&object->transform, &object->scale);

    struct vec3 scale_inv;
    scale_inv.x = 1.0f / object->scale.x;
    scale_inv.y = 1.0f / object->scale.y;
    scale_inv.z = 1.0f / object->scale.z;

    struct vec3 trans_inv = object->trans;
    v3_negate(&trans_inv);

    object->transform_inverse = MAT4_IDENT;
    mat4_scale(&object->transform_inverse, &scale_inv);
    mat4_rotz(&object->transform_inverse, -object->rot.z);
    mat4_roty(&object->transform_inverse, -object->rot.y);
    mat4_rotx(&object->transform_inverse, -object->rot.x);
    mat4_translate(&object->transform_inverse, &trans_inv);

    //struct mat4 mm = object->transform;
    //mat4_mul(&mm, &object->transform_inverse);
    //RICO_ASSERT(mat4_equals(&mm, &MAT4_IDENT));
}

const struct mat4 *object_transform_get(struct rico_object *object)
{
    return &object->transform;
}

bool object_collide_ray(float *_dist, struct rico_object *object,
                        const struct ray *ray)
{
    return collide_ray_obb(_dist, ray, &object->bbox, &object->transform,
                           &object->transform_inverse);
}

bool object_collide_ray_type(struct rico_object **_object, float *_dist,
                             enum rico_obj_type type, const struct ray *ray)
{
    bool collided = false;
    float distance;
    *_dist = Z_FAR; // Track closest object

    struct rico_pool *pool = chunk_pool(chunk_active, RICO_HND_OBJECT);
    struct rico_object *obj;
    for (u32 i = 0; i < pool->active; ++i)
    {
        obj = (struct rico_object *)pool->handles[i];
        if (obj->type == type)
        {
            collided = collide_ray_obb(&distance, ray, &obj->bbox,
                                       &obj->transform,
                                       &obj->transform_inverse);

            // If closest so far, save info
            if (collided && distance < *_dist)
            {
                // Record object handle and distance
                *_object = (struct rico_object *)pool->handles[i];
                *_dist = distance;
            }
        }
    }

    return collided;
}

void object_render(struct rico_object *object, const struct camera *camera)
{
    if (object->type == OBJ_STATIC)
    {
        glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Bind material and render mesh
    material_bind(object->material);
    mesh_render(object->mesh);

    // Clean up
    material_unbind(object->material);
}

void object_render_type(enum rico_obj_type type,
                        const struct program_default *prog,
                        const struct camera *camera)
{
    glUseProgram(prog->prog_id);

    // Model transform
    struct mat4 proj_matrix = camera->proj_matrix;
    struct mat4 view_matrix = camera->view_matrix;

    // TODO: Get the light out of here!!! It should't be updating its position
    //       in the render function, argh!
    u32 ticks = SDL_GetTicks();
    float light_pos_x = cosf((float)ticks / 2000.0f) * 16.0f;
    float light_pos_z = sinf((float)ticks / 2000.0f) * 16.0f;

    struct light_point light;
    light.color    = (struct vec3) { 1.0f, 0.9f, 0.6f };
    light.position = (struct vec3) { light_pos_x, 3.0f, light_pos_z};
    //light.ambient  = (struct vec3) { 0.17f, 0.17f, 0.19f };
    light.ambient  = (struct vec3){ 0.01f, 0.01f, 0.01f };
    light.kc = 1.0f;
    light.kl = 0.05f;
    light.kq = 0.001f;

    if (type == OBJ_STRING_SCREEN)
    {
        // TODO: Create dedicated shader for OBJ_STRING_SCREEN instead of using
        //       all of these hacks to disable scaling, projection, lighting...
        proj_matrix = MAT4_IDENT;
        view_matrix = MAT4_IDENT;

        light.color   = VEC3_ONE;
        light.ambient = VEC3_ONE;

        light.kc = 1.0f;
        light.kl = 0.0f;
        light.kq = 0.0f;
    }

    glUniformMatrix4fv(prog->u_proj, 1, GL_TRUE, proj_matrix.a);
    glUniformMatrix4fv(prog->u_view, 1, GL_TRUE, view_matrix.a);
    glUniform3fv(prog->u_view_pos, 1, (const GLfloat *)&camera->position);

    // Material textures
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    glUniform1i(prog->u_material_diff, 0);
    glUniform1i(prog->u_material_spec, 1);

    // Lighting
    glUniform3fv(prog->u_light_position, 1, (const GLfloat *)&light.position);
    glUniform3fv(prog->u_light_ambient,  1, (const GLfloat *)&light.ambient);
    glUniform3fv(prog->u_light_color,    1, (const GLfloat *)&light.color);
    glUniform1f(prog->u_light_kc, light.kc);
    glUniform1f(prog->u_light_kl, light.kl);
    glUniform1f(prog->u_light_kq, light.kq);

    struct rico_pool *pool = chunk_pool(chunk_active, RICO_HND_OBJECT);
    struct rico_object *obj;
    for (u32 i = 0; i < pool->active; ++i)
    {
        obj = (struct rico_object *)pool->handles[i];
        if (obj->type != type)
            continue;

        glUseProgram(prog->prog_id);

        ////////////////////////////////////////////////////////////////////
        // Set object-specific uniform values
        ////////////////////////////////////////////////////////////////////

        // UV-coord scale
        // HACK: This only works when object is uniformly scaled on X/Y
        //       plane.
        // TODO: UV scaling in general only works when object is uniformly
        //       scaled. Maybe I should only allow textured objects to be
        //       uniformly scaled?
        if (type == OBJ_STRING_WORLD || type == OBJ_STRING_SCREEN)
        {
            glUniform2f(prog->u_scale_uv, 1.0f, 1.0f);
        }
        else
        {
            glUniform2f(prog->u_scale_uv, obj->scale.x, obj->scale.y);
        }

        // Model matrix
        glUniformMatrix4fv(prog->u_model, 1, GL_TRUE, obj->transform.a);

        // Model material shiny
        glUniform1f(prog->u_material_shiny, material_shiny(obj->material));

        // Render object
        object_render(obj, camera);

        // TODO: Batch bounding boxes
        // Render bbox
        if (is_edit_state(state_get()))
            prim_draw_bbox(&obj->bbox, &obj->transform);
    }

    glUseProgram(0);
}

int object_print(struct rico_object *object)
{
    enum rico_error err;

    // TODO: Free previous "[object_print]" string, look up by name

    // Print to screen
    char buf[256] = { 0 };
    object_to_string(object, buf, sizeof(buf));
    err = string_init("[object_print]", STR_SLOT_SELECTED_OBJ, 0, FONT_HEIGHT, COLOR_GRAY_HIGHLIGHT, 0,
                      NULL, buf);
    return err;
}

void object_to_string(struct rico_object *object, char *buf, int buf_count)
{
    int len;

    if (!object)
    {
        len = snprintf(buf, buf_count,
            "     UID: %d\n" \
            "    Name: --\n" \
            "    Type: --\n" \
            "   Trans: --\n" \
            "     Rot: --\n" \
            "   Scale: --\n" \
            "    Mesh: --\n" \
            "Material: --\n",
            UID_NULL
        );
    }
    else
    {
        len = snprintf(buf, buf_count,
            "     UID: %d\n"       \
            "    Name: %s\n"       \
            "    Type: %s\n"       \
            "   Trans: %f %f %f\n" \
            "     Rot: %f %f %f\n" \
            "   Scale: %f %f %f\n" \
            "    Mesh: %d %s\n"    \
            "Material: %d %s\n",
            object->hnd.uid,
            object->hnd.name,
            rico_obj_type_string[object->type],
            object->trans.x, object->trans.y, object->trans.z,
            object->rot.x,   object->rot.y,   object->rot.z,
            object->scale.x, object->scale.y, object->scale.z,
            object->mesh->hnd.uid,     object->mesh->hnd.name,
            object->material->hnd.uid, object->material->hnd.name
        );
    }

    string_truncate(buf, buf_count, len);
}

#if 0
//int object_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(object_serialize_0)
{
    const struct rico_object *obj = handle;
    fwrite(&object->type,     sizeof(object->type),     1, file->fs);
    fwrite(&object->trans,    sizeof(object->trans),    1, file->fs);
    fwrite(&object->rot,      sizeof(object->rot),      1, file->fs);
    fwrite(&object->scale,    sizeof(object->scale),    1, file->fs);
    fwrite(&object->mesh,     sizeof(object->mesh),     1, file->fs);
    fwrite(&object->material, sizeof(object->material), 1, file->fs);
    return rico_serialize(&object->bbox, file);
}

//int object_deserialize_0(void *_handle, const struct rico_file *file)
DESERIAL(object_deserialize_0)
{
    enum rico_error err;
    u32 mesh, material;

    struct rico_object *obj = *_handle;
    fread(&object->type,  sizeof(object->type),     1, file->fs);
    fread(&object->trans, sizeof(object->trans),    1, file->fs);
    fread(&object->rot,   sizeof(object->rot),      1, file->fs);
    fread(&object->scale, sizeof(object->scale),    1, file->fs);
    fread(&mesh,       sizeof(object->mesh),     1, file->fs);
    fread(&material,   sizeof(object->material), 1, file->fs);

    update_transform(obj);
    object->mesh     = mesh_request(mesh);
    object->material = material_request(material);

    struct bbox *bbox;
    err = rico_deserialize(&bbox, file);
    if (err == ERR_SERIALIZE_DISABLED)
        object->bbox = *mesh_bbox(object->mesh);
    else
        object->bbox = *bbox;

    object->bbox.wireframe = true;
    return err;
}
#endif
