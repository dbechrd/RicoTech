const char *rico_obj_type_string[] = {
    RICO_OBJ_TYPES(GEN_STRING)
};

global const char *object_name(struct rico_object *obj)
{
    return (u8 *)obj + obj->name_offset;
}

global struct rico_object *object_copy(struct pack *pack,
                                       struct rico_object *other,
                                       const char *name)
{
    // Create new object with same mesh / texture
    u32 new_obj_id = load_object(pack, name, other->type, other->mesh_id,
                                 other->material_id, &other->bbox);
    struct rico_object *new_obj = pack_read(pack, new_obj_id);

    // TODO: Make transform one property and add optional param to object_init
    // Copy transform
    object_trans_set(new_obj, &other->trans);
    object_rot_set(new_obj, &other->rot);
    object_scale_set(new_obj, &other->scale);

    return new_obj;
}

#if 0
int object_free(struct rico_object *object)
{
    enum rico_error err;

#if RICO_DEBUG_OBJECT
    printf("[ obj][free] uid=%d name=%s\n", object->hnd.uid, object->hnd.name);
#endif

    err = pool_remove(object->hnd.pool, object->hnd.id);
    return err;

}

void object_free_all(struct rico_chunk *chunk)
{
    struct rico_pool *pool = chunk->pools[RICO_HND_OBJECT];

    u8 *block = pool_last(pool);
    while (block)
    {
        object_free((struct rico_object *)block);
        block = pool_prev(pool, block);
    }
}
#endif

void object_bbox_recalculate_all(struct pack *pack)
{
    struct rico_object *obj = 0;
    for (u32 blob = 1; blob < pack->blob_count; ++blob)
    {
        if (pack->index[blob].type != OBJ_STATIC)
            continue;

        obj = pack_read(pack, blob);
        struct rico_mesh *mesh = pack_read(pack, obj->mesh_id);
        obj->bbox = mesh->bbox;
    }
}

bool object_selectable(struct rico_object *object)
{
    return (object->type != OBJ_STRING_SCREEN);
}

void object_select(struct rico_object *object)
{
    object->bbox.wireframe = false;
}

void object_deselect(struct rico_object *object)
{
    object->bbox.wireframe = true;
}

global void object_update_transform(struct rico_object *object)
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

void object_trans(struct rico_object *object, const struct vec3 *v)
{
    v3_add(&object->trans, v);
    object_update_transform(object);
}

const struct vec3 *object_trans_get(struct rico_object *object)
{
    return &object->trans;
}

void object_trans_set(struct rico_object *object,
                      const struct vec3 *v)
{
    object->trans = *v;
    object_update_transform(object);
}

void object_rot(struct rico_object *object, const struct vec3 *v)
{
    v3_add(&object->rot, v);
    object_update_transform(object);
}

void object_rot_set(struct rico_object *object, const struct vec3 *v)
{
    object->rot = *v;
    object_update_transform(object);
}

const struct vec3 *object_rot_get(struct rico_object *object)
{
    return &object->rot;
}

void object_rot_x(struct rico_object *object, float deg)
{
    object->rot.x += deg;
    object_update_transform(object);
}

void object_rot_x_set(struct rico_object *object, float deg)
{
    object->rot.x = deg;
    object_update_transform(object);
}

void object_rot_y(struct rico_object *object, float deg)
{
    object->rot.y += deg;
    object_update_transform(object);
}

void object_rot_y_set(struct rico_object *object, float deg)
{
    object->rot.y = deg;
    object_update_transform(object);
}

void object_rot_z(struct rico_object *object, float deg)
{
    object->rot.z += deg;
    object_update_transform(object);
}

void object_rot_z_set(struct rico_object *object, float deg)
{
    object->rot.z = deg;
    object_update_transform(object);
}

void object_scale(struct rico_object *object, const struct vec3 *v)
{
    v3_add(&object->scale, v);
    object_update_transform(object);
}

void object_scale_set(struct rico_object *object,
                      const struct vec3 *v)
{
    object->scale = *v;
    object_update_transform(object);
}

const struct vec3 *object_scale_get(struct rico_object *object)
{
    return &object->scale;
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

bool object_collide_ray_type(struct rico_chunk *chunk,
                             struct rico_object **_object, float *_dist,
                             enum rico_obj_type type, const struct ray *ray)
{
    bool collided = false;
    float distance;
    *_dist = Z_FAR; // Track closest object

    struct rico_pool *pool = chunk->pools[RICO_HND_OBJECT];
    struct rico_object *obj = pool_first(pool);
    while (obj)
    {
        if (obj->type == type)
        {
            collided = collide_ray_obb(&distance, ray, &obj->bbox,
                                       &obj->transform,
                                       &obj->transform_inverse);

            // If closest so far, save info
            if (collided && distance < *_dist)
            {
                // Record object handle and distance
                *_object = obj;
                *_dist = distance;
            }
        }
        obj = pool_next(pool, obj);
    }

    return collided;
}

void object_render_type(struct pack *pack, enum rico_obj_type type,
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
    light.ambient  = (struct vec3){ 0.10f, 0.09f, 0.11f };
    light.kc = 1.0f;
    light.kl = 0.05f;
    light.kq = 0.001f;

    if (type == OBJ_STRING_SCREEN)
    {
        // TODO: Create dedicated shader for OBJ_STRING_SCREEN instead of using
        //       all of these hacks to disable scaling, projection, lighting...
        proj_matrix = MAT4_IDENT;
        view_matrix = MAT4_IDENT;

        light.color    = VEC3_ONE;
        light.position = VEC3_ZERO;
        light.ambient  = VEC3_ONE;

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

    //struct rico_pool *pool = chunk->pools[RICO_HND_OBJECT];
    //struct rico_object *obj = pool_first(pool);
    struct rico_object *obj = 0;
    for (u32 blob = 1; blob < pack->blobs_used; ++blob)
    {
        if (pack->index[blob].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, blob);
        if (obj->type != type)
            continue;

        if (obj->type == OBJ_STATIC)
        {
            glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glUseProgram(prog->prog_id);

        // Set object-specific uniform values

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

        // Bind material
        material_bind(pack, obj->material_id, prog->u_material_shiny);

        // Render
        mesh_render(pack, obj->mesh_id);

        // Clean up
        material_unbind(pack, obj->material_id);

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

    err = string_free_slot(STR_SLOT_SELECTED_OBJ);
    if (err) return err;

    int len;
    char buf[BFG_MAXSTRING + 1] = { 0 };

    if (object)
    {
        struct rico_mesh *mesh = pack_read(pack_active, object->mesh_id);
        struct rico_material *material = pack_read(pack_active,
                                                   object->material_id);

        len = snprintf(buf, sizeof(buf),
            "     UID: %d\n"       \
            "    Name: %s\n"       \
            "    Type: %s\n"       \
            "   Trans: %f %f %f\n" \
            "     Rot: %f %f %f\n" \
            "   Scale: %f %f %f\n" \
            "    Mesh: %d %s\n"    \
            "    BBox: %f %f %f\n" \
            "          %f %f %f\n" \
            "Material: %d %s\n",
            object->id,
            object_name(object),
            rico_obj_type_string[object->type],
            object->trans.x, object->trans.y, object->trans.z,

            object->rot.x,   object->rot.y,   object->rot.z,
            object->scale.x, object->scale.y, object->scale.z,
            (mesh) ? 777 : 0,
            (mesh) ? mesh_name(mesh) : "ID_NULL",
            object->bbox.p[0].x, object->bbox.p[0].y, object->bbox.p[0].z,
            object->bbox.p[1].x, object->bbox.p[1].y, object->bbox.p[1].z,
            (material) ? 777 : 0,
            (material) ? material_name(material) : "ID_NULL"
        );
    }
    else
    {
        len = snprintf(buf, sizeof(buf), "No object selected");
        /*
        len = snprintf(buf, sizeof(buf),
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
        */
    }

    string_truncate(buf, sizeof(buf), len);

    struct rico_string *str;
    err = chunk_alloc((void **)&str, chunk_transient, RICO_HND_STRING);
    if (err) return err;
    err = string_init(str, rico_string_slot_string[STR_SLOT_SELECTED_OBJ],
                      STR_SLOT_SELECTED_OBJ, 0, FONT_HEIGHT,
                      COLOR_DARK_GRAY_HIGHLIGHT, 0, NULL, buf);
    return err;
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
