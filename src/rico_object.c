const char *rico_obj_type_string[] = {
    RICO_OBJ_TYPES(GEN_STRING)
};

struct hnd RICO_DEFAULT_OBJECT = { 0 };

internal void update_transform(struct rico_object *obj);

internal inline struct rico_pool **object_pool_ptr(enum rico_persistence persist)
{
    struct rico_chunk *chunk = chunk_active();
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->pools[persist][POOL_OBJECTS]);
    return &chunk->pools[persist][POOL_OBJECTS];
}

internal inline struct rico_pool *object_pool(enum rico_persistence persist)
{
    return *object_pool_ptr(persist);
}

internal inline struct rico_object *object_find(struct hnd handle)
{
    struct rico_object *object = pool_read(object_pool(handle.persist),
                                           handle.value);
    RICO_ASSERT(object->uid.uid);
    return object;
}

int object_request_by_name(struct hnd *_handle, enum rico_persist persist,
                           const char *name)
{
    struct hnd handle = hashtable_search_by_name(&global_objects, name);
    if (!handle.value)
    {
        return RICO_ERROR(ERR_OBJECT_INVALID_NAME, "Object not found: %s.",
                          name);
    }

    *_handle = handle;
    return SUCCESS;
}

int object_create(struct hnd *_handle, enum rico_persist persist,
                  const char *name, enum rico_obj_type type, struct hnd mesh,
                  struct hnd material, const struct bbox *bbox,
                  bool serialize)
{
    enum rico_error err;

#if RICO_DEBUG_OBJECT
    printf("[ obj][init] name=%s\n", name);
#endif

    struct hnd handle;
    struct rico_object *obj;
    err = pool_handle_alloc(object_pool_ptr(persist), &handle, &obj);
    if (err) return err;

    uid_init(&obj->uid, RICO_UID_OBJECT, name, serialize);
    obj->type = type;
    obj->trans = VEC3_ZERO;
    obj->rot = VEC3_ZERO;
    if (type == OBJ_STRING_SCREEN)
        obj->scale = VEC3_SCALE_ASPECT;
    else
        obj->scale = VEC3_ONE;
    obj->transform = MAT4_IDENT;
    obj->transform_inverse = MAT4_IDENT;
    obj->mesh = mesh_request(mesh);
    obj->material = material_request(material);
    obj->bbox = (bbox != NULL) ? *bbox : *mesh_bbox(obj->mesh);

    update_transform(obj);

    if (_handle) *_handle = handle;
    return err;
}

int object_copy(struct hnd *_handle, struct hnd handle,
                const char *name)
{
    enum rico_error err;
    struct rico_object *obj = object_find(handle);

    // Create new object with same mesh / texture
    err = object_create(_handle, handle.persist, name, obj->type, obj->mesh,
                        obj->material, NULL, true);
    if (err) return err;

    // Copy transform
    object_trans_set(*_handle, &obj->trans);
    object_rot_set(*_handle, &obj->rot);
    object_scale_set(*_handle, &obj->scale);

    return err;
}

void object_free(struct hnd handle)
{
    struct rico_object *obj = object_find(handle);

    // Cleanup: Use fixed pool slots
    //if (handle.value == RICO_DEFAULT_OBJECT)
    //    return;

#if RICO_DEBUG_OBJECT
    printf("[ obj][free] uid=%d name=%s\n", obj->uid.uid, obj->uid.name);
#endif

    mesh_free(obj->mesh);
    material_free(obj->material);

    obj->uid.uid = UID_NULL;
    pool_handle_free(object_pool(handle.persist), handle);
}

void object_free_all(enum rico_persist persist)
{
    for (int i = object_pool(persist)->active - 1; i >= 0; --i)
    {
        object_free(object_pool(persist)->handles[i]);
    }
}

void object_bbox_recalculate_all()
{
    struct rico_pool *pool;
    for (u32 persist = 0; persist < PERSIST_COUNT; ++persist)
    {
        pool = object_pool(persist);
        for (u32 i = 0; i < pool->active; ++i)
        {
            object_bbox_set(pool->handles[i], NULL);
        }
    }
}

void object_bbox_set(struct hnd handle,
                     const struct bbox *bbox)
{
    struct rico_object *obj = object_find(handle);
    obj->bbox = (bbox != NULL) ? *bbox : *mesh_bbox(obj->mesh);
}

void object_mesh_set(struct hnd handle, struct hnd mesh,
                     const struct bbox *bbox)
{
    struct rico_object *obj = object_find(handle);
    mesh_free(obj->mesh);
    obj->mesh = mesh_request(mesh);
    obj->bbox = (bbox != NULL) ? *bbox : *mesh_bbox(obj->mesh);
}

void object_mesh_next(struct hnd handle)
{
    struct rico_object *obj = object_find(handle);

    struct hnd next_mesh = mesh_next(obj->mesh);
    if (next_mesh.value == obj->mesh.value)
        return;

    mesh_free(obj->mesh);
    obj->mesh = mesh_request(next_mesh);
    obj->bbox = *mesh_bbox(next_mesh);
}

void object_mesh_prev(struct hnd handle)
{
    struct rico_object *obj = object_find(handle);

    struct hnd prev_mesh = mesh_prev(obj->mesh);
    if (prev_mesh.value == obj->mesh.value)
        return;

    mesh_free(obj->mesh);
    obj->mesh = mesh_request(prev_mesh);
    obj->bbox = *mesh_bbox(prev_mesh);
}

void object_material_set(struct hnd handle, struct hnd material)
{
    struct rico_object *obj = object_find(handle);
    material_free(obj->material);
    obj->material = material_request(material);
}

enum rico_obj_type object_type_get(struct hnd handle)
{
    if (!handle.value)
        return OBJ_NULL;

    struct rico_object *obj = object_find(handle);
    return obj->type;
}

bool object_selectable(struct hnd handle)
{
    enum rico_obj_type type = object_type_get(handle);
    return (type != OBJ_NULL &&
            type != OBJ_STRING_SCREEN);
}

struct hnd object_next(struct hnd handle)
{
    struct hnd start = pool_handle_next(object_pool(handle.persist), handle);
    struct hnd next = start;

    do
    {
        if (object_selectable(next))
            return next;

        next = pool_handle_next(object_pool(handle.persist), next);
    } while (next.value != start.value);

    return HANDLE_NULL;
}

struct hnd object_prev(struct hnd handle)
{
    struct hnd start = pool_handle_prev(object_pool(handle.persist), handle);
    struct hnd prev = start;

    do
    {
        if (object_selectable(prev))
            return prev;

        prev = pool_handle_prev(object_pool(handle.persist), prev);
    } while (prev.value != start.value);

    return HANDLE_NULL;
}

void object_select(struct hnd handle)
{
    struct rico_object *obj = object_find(handle);
    obj->bbox.wireframe = false;
}

void object_deselect(struct hnd handle)
{
    struct rico_object *obj = object_find(handle);
    obj->bbox.wireframe = true;
}

void object_trans(struct hnd handle, const struct vec3 *v)
{
    struct rico_object *obj = object_find(handle);
    v3_add(&obj->trans, v);
    update_transform(obj);
}

const struct vec3 *object_trans_get(struct hnd handle)
{
    struct rico_object *obj = object_find(handle);
    return &obj->trans;
}

void object_trans_set(struct hnd handle,
                      const struct vec3 *v)
{
    struct rico_object *obj = object_find(handle);
    obj->trans = *v;
    update_transform(obj);
}

void object_rot(struct hnd handle, const struct vec3 *v)
{
    struct rico_object *obj = object_find(handle);
    v3_add(&obj->rot, v);
    update_transform(obj);
}

void object_rot_set(struct hnd handle, const struct vec3 *v)
{
    struct rico_object *obj = object_find(handle);
    obj->rot = *v;
    update_transform(obj);
}

const struct vec3 *object_rot_get(struct hnd handle)
{
    struct rico_object *obj = object_find(handle);
    return &obj->rot;
}

void object_rot_x(struct hnd handle, float deg)
{
    struct rico_object *obj = object_find(handle);
    obj->rot.x += deg;
    update_transform(obj);
}

void object_rot_x_set(struct hnd handle, float deg)
{
    struct rico_object *obj = object_find(handle);
    obj->rot.x = deg;
    update_transform(obj);
}

void object_rot_y(struct hnd handle, float deg)
{
    struct rico_object *obj = object_find(handle);
    obj->rot.y += deg;
    update_transform(obj);
}

void object_rot_y_set(struct hnd handle, float deg)
{
    struct rico_object *obj = object_find(handle);
    obj->rot.y = deg;
    update_transform(obj);
}

void object_rot_z(struct hnd handle, float deg)
{
    struct rico_object *obj = object_find(handle);
    obj->rot.z += deg;
    update_transform(obj);
}

void object_rot_z_set(struct hnd handle, float deg)
{
    struct rico_object *obj = object_find(handle);
    obj->rot.z = deg;
    update_transform(obj);
}

void object_scale(struct hnd handle, const struct vec3 *v)
{
    struct rico_object *obj = object_find(handle);
    v3_add(&obj->scale, v);
    update_transform(obj);
}

void object_scale_set(struct hnd handle,
                      const struct vec3 *v)
{
    struct rico_object *obj = object_find(handle);
    obj->scale = *v;
    update_transform(obj);
}

const struct vec3 *object_scale_get(struct hnd handle)
{
    struct rico_object *obj = object_find(handle);
    return &obj->scale;
}

internal void update_transform(struct rico_object *obj)
{
    //HACK: Order of these operations might not always be the same.. should
    //      probably just store the transformation matrix directly rather than
    //      trying to figure out which order to do what. Unfortunately, doing
    //      this makes relative transformations very difficult. Maybe objects
    //      should have "edit mode" where the matrix is decomposed, then
    //      recomposed again when edit mode is ended?
    obj->transform = MAT4_IDENT;
    mat4_translate(&obj->transform, &obj->trans);
    mat4_rotx(&obj->transform, obj->rot.x);
    mat4_roty(&obj->transform, obj->rot.y);
    mat4_rotz(&obj->transform, obj->rot.z);
    mat4_scale(&obj->transform, &obj->scale);

    struct vec3 scale_inv;
    scale_inv.x = 1.0f / obj->scale.x;
    scale_inv.y = 1.0f / obj->scale.y;
    scale_inv.z = 1.0f / obj->scale.z;

    struct vec3 trans_inv = obj->trans;
    v3_negate(&trans_inv);

    obj->transform_inverse = MAT4_IDENT;
    mat4_scale(&obj->transform_inverse, &scale_inv);
    mat4_rotz(&obj->transform_inverse, -obj->rot.z);
    mat4_roty(&obj->transform_inverse, -obj->rot.y);
    mat4_rotx(&obj->transform_inverse, -obj->rot.x);
    mat4_translate(&obj->transform_inverse, &trans_inv);

    //struct mat4 mm = obj->transform;
    //mat4_mul(&mm, &obj->transform_inverse);
    //RICO_ASSERT(mat4_equals(&mm, &MAT4_IDENT));
}

const struct mat4 *object_transform_get(struct hnd handle)
{
    struct rico_object *obj = object_find(handle);
    return &obj->transform;
}

bool object_collide_ray(float *_dist, struct hnd handle,
                        const struct ray *ray)
{
    struct rico_object *obj = object_find(handle);
    return collide_ray_obb(_dist, ray, &obj->bbox, &obj->transform,
                           &obj->transform_inverse);
}

bool object_collide_ray_type(struct hnd *_handle, float *_dist,
                             enum rico_obj_type type, const struct ray *ray)
{
    bool collided = false;
    float distance;
    *_dist = Z_FAR; // Track closest object

    struct rico_object *obj;
    for (u32 persist = 0; persist < PERSIST_COUNT; ++persist)
    {
        for (u32 i = 0; i < object_pool(persist)->active; ++i)
        {
            obj = object_find(object_pool(persist)->handles[i]);
            if (obj->type == type)
            {
                collided = collide_ray_obb(&distance, ray, &obj->bbox,
                                           &obj->transform,
                                           &obj->transform_inverse);

                // If closest so far, save info
                if (collided && distance < *_dist)
                {
                    // Record object handle and distance
                    *_handle = object_pool(persist)->handles[i];
                    *_dist = distance;
                }
            }
        }
    }

    return collided;
}

void object_render(struct hnd handle, const struct camera *camera)
{
    struct rico_object *obj = object_find(handle);

    if (obj->type == OBJ_STATIC)
    {
        glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }


    // Bind material and render mesh
    material_bind(obj->material);
    mesh_render(obj->mesh);

    // Clean up
    material_unbind(obj->material);
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

    struct rico_object *obj;
    for (u32 persist = 0; persist < PERSIST_COUNT; ++persist)
    {
        for (u32 i = 0; i < object_pool(persist)->active; ++i)
        {
            obj = object_find(object_pool(persist)->handles[i]);
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
            object_render(object_pool(persist)->handles[i], camera);

            // TODO: Batch bounding boxes
            // Render bbox
            if (is_edit_state(state_get()))
                prim_draw_bbox(&obj->bbox, &obj->transform);
        }
    }

    glUseProgram(0);
}

int object_print(struct hnd handle, enum rico_string_slot slot)
{
    enum rico_error err;

    // Print to screen
    char buf[256] = { 0 };
    object_to_string(handle, buf, sizeof(buf));
    err = string_init(TRANSIENT, rico_string_slot_string[slot], slot, 0,
                      FONT_HEIGHT, COLOR_GRAY_HIGHLIGHT, 0, HANDLE_NULL, buf);
    return err;
}

void object_to_string(struct hnd handle, char *buf, int buf_count)
{
    int len;

    if (!handle.value)
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
            UID_NULL);
    }
    else
    {
        struct rico_object *obj = object_find(handle);
        len = snprintf(buf, buf_count,
            "     UID: %d\n"       \
            "    Name: %s\n"       \
            "    Type: %s\n"       \
            "   Trans: %f %f %f\n" \
            "     Rot: %f %f %f\n" \
            "   Scale: %f %f %f\n" \
            "    Mesh: %d %s\n"    \
            "Material: %d %s\n",
            obj->uid.uid,
            obj->uid.name,
            rico_obj_type_string[obj->type],
            obj->trans.x,  obj->trans.y, obj->trans.z,
            obj->rot.x,    obj->rot.y,   obj->rot.z,
            obj->scale.x,  obj->scale.y, obj->scale.z,
            obj->mesh.value,     mesh_name(obj->mesh),
            obj->material.value, material_name(obj->material));
    }

    string_truncate(buf, buf_count, len);
}

#if 0
//int object_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(object_serialize_0)
{
    const struct rico_object *obj = handle;
    fwrite(&obj->type,     sizeof(obj->type),     1, file->fs);
    fwrite(&obj->trans,    sizeof(obj->trans),    1, file->fs);
    fwrite(&obj->rot,      sizeof(obj->rot),      1, file->fs);
    fwrite(&obj->scale,    sizeof(obj->scale),    1, file->fs);
    fwrite(&obj->mesh,     sizeof(obj->mesh),     1, file->fs);
    fwrite(&obj->material, sizeof(obj->material), 1, file->fs);
    return rico_serialize(&obj->bbox, file);
}

//int object_deserialize_0(void *_handle, const struct rico_file *file)
DESERIAL(object_deserialize_0)
{
    enum rico_error err;
    u32 mesh, material;

    struct rico_object *obj = *_handle;
    fread(&obj->type,  sizeof(obj->type),     1, file->fs);
    fread(&obj->trans, sizeof(obj->trans),    1, file->fs);
    fread(&obj->rot,   sizeof(obj->rot),      1, file->fs);
    fread(&obj->scale, sizeof(obj->scale),    1, file->fs);
    fread(&mesh,       sizeof(obj->mesh),     1, file->fs);
    fread(&material,   sizeof(obj->material), 1, file->fs);

    update_transform(obj);
    obj->mesh     = mesh_request(mesh);
    obj->material = material_request(material);

    struct bbox *bbox;
    err = rico_deserialize(&bbox, file);
    if (err == ERR_SERIALIZE_DISABLED)
        obj->bbox = *mesh_bbox(obj->mesh);
    else
        obj->bbox = *bbox;

    obj->bbox.wireframe = true;
    return err;
}
#endif