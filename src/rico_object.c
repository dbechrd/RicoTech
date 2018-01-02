const char *rico_obj_type_string[] = {
    RICO_OBJ_TYPES(GEN_STRING)
};

global const char *object_name(struct rico_object *obj)
{
    RICO_ASSERT(obj->name_offset);
    return (char *)((u8 *)obj + obj->name_offset);
}
global u32 *object_meshes(struct rico_object *obj)
{
    RICO_ASSERT(obj->meshes_offset);
    return (u32 *)((u8 *)obj + obj->meshes_offset);
}
global u32 *object_materials(struct rico_object *obj)
{
    RICO_ASSERT(obj->materials_offset);
    return (u32 *)((u8 *)obj + obj->materials_offset);
}
global u32 object_mesh(struct rico_object *obj)
{
    return (obj->mesh_count)
        ? object_meshes(obj)[obj->mesh_idx]
        : 0;
}
global u32 object_material(struct rico_object *obj)
{
    return (obj->material_count)
        ? object_materials(obj)[obj->material_idx]
        : 0;
}

global struct rico_object *object_copy(struct pack *pack,
                                       struct rico_object *other,
                                       const char *name)
{
    // Create new object with same mesh / texture
    u32 new_obj_id = load_object(pack, name, other->type, other->mesh_count,
                                 object_meshes(other), other->material_count,
                                 object_materials(other), &other->bbox);
    struct rico_object *new_obj = pack_lookup(pack, new_obj_id);

    // TODO: Make transform one property and add optional param to object_init
    // Copy transform
    object_trans_set(new_obj, &other->xform.trans);
    object_rot_set(new_obj, &other->xform.rot);
    object_scale_set(new_obj, &other->xform.scale);

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
    struct rico_object *obj;
    struct rico_mesh *mesh;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type != OBJ_STATIC)
            continue;

        if (obj->mesh_count)
        {
            mesh = pack_lookup(pack, object_meshes(obj)[0]);
        }
        else
        {
            mesh = pack_lookup(pack_default, MESH_DEFAULT_BBOX);
        }
        obj->bbox = mesh->bbox;
    }
}

bool object_selectable(struct rico_object *object)
{
    return (object->type != OBJ_STRING_SCREEN);
}

void object_select_toggle(struct rico_object *object)
{
    object->bbox.wireframe = !object->bbox.wireframe;;
}

void object_select(struct rico_object *object)
{
    RICO_ASSERT(object_selectable(object));
    object->bbox.wireframe = false;
}

void object_deselect(struct rico_object *object)
{
    object->bbox.wireframe = true;
}

global void object_transform_update(struct rico_object *object)
{
    //HACK: Order of these operations might not always be the same.. should
    //      probably just store the transformation matrix directly rather than
    //      trying to figure out which order to do what. Unfortunately, doing
    //      this makes relative transformations very difficult. Maybe objects
    //      should have "edit mode" where the matrix is decomposed, then
    //      recomposed again when edit mode is ended?
    struct mat4 transform = MAT4_IDENT;
    mat4_translate(&transform, &object->xform.trans);
    mat4_rotx(&transform, object->xform.rot.x);
    mat4_roty(&transform, object->xform.rot.y);
    mat4_rotz(&transform, object->xform.rot.z);
    mat4_scale(&transform, &object->xform.scale);
    object->xform.matrix = transform;

    struct vec3 scale_inv;
    scale_inv.x = 1.0f / object->xform.scale.x;
    scale_inv.y = 1.0f / object->xform.scale.y;
    scale_inv.z = 1.0f / object->xform.scale.z;

    struct vec3 trans_inv = object->xform.trans;
    v3_negate(&trans_inv);

    struct mat4 transform_inverse = MAT4_IDENT;
    mat4_scale(&transform_inverse, &scale_inv);
    mat4_rotz(&transform_inverse, -object->xform.rot.z);
    mat4_roty(&transform_inverse, -object->xform.rot.y);
    mat4_rotx(&transform_inverse, -object->xform.rot.x);
    mat4_translate(&transform_inverse, &trans_inv);
    object->xform.matrix_inverse = transform_inverse;

    //struct mat4 mm = object->transform;
    //mat4_mul(&mm, &object->transform_inverse);
    //RICO_ASSERT(mat4_equals(&mm, &MAT4_IDENT));
}

void object_trans(struct rico_object *object, const struct vec3 *v)
{
    v3_add(&object->xform.trans, v);
    object_transform_update(object);
}

const struct vec3 *object_trans_get(struct rico_object *object)
{
    return &object->xform.trans;
}

void object_trans_set(struct rico_object *object,
                      const struct vec3 *v)
{
    object->xform.trans = *v;
    object_transform_update(object);
}

void object_rot(struct rico_object *object, const struct vec3 *v)
{
    v3_add(&object->xform.rot, v);
    object_transform_update(object);
}

void object_rot_set(struct rico_object *object, const struct vec3 *v)
{
    object->xform.rot = *v;
    object_transform_update(object);
}

const struct vec3 *object_rot_get(struct rico_object *object)
{
    return &object->xform.rot;
}

void object_rot_x(struct rico_object *object, float deg)
{
    object->xform.rot.x += deg;
    object_transform_update(object);
}

void object_rot_x_set(struct rico_object *object, float deg)
{
    object->xform.rot.x = deg;
    object_transform_update(object);
}

void object_rot_y(struct rico_object *object, float deg)
{
    object->xform.rot.y += deg;
    object_transform_update(object);
}

void object_rot_y_set(struct rico_object *object, float deg)
{
    object->xform.rot.y = deg;
    object_transform_update(object);
}

void object_rot_z(struct rico_object *object, float deg)
{
    object->xform.rot.z += deg;
    object_transform_update(object);
}

void object_rot_z_set(struct rico_object *object, float deg)
{
    object->xform.rot.z = deg;
    object_transform_update(object);
}

void object_scale(struct rico_object *object, const struct vec3 *v)
{
    v3_add(&object->xform.scale, v);
    object_transform_update(object);
}

void object_scale_set(struct rico_object *object,
                      const struct vec3 *v)
{
    object->xform.scale = *v;
    object_transform_update(object);
}

const struct vec3 *object_scale_get(struct rico_object *object)
{
    return &object->xform.scale;
}

const struct mat4 *object_matrix_get(struct rico_object *object)
{
    return &object->xform.matrix;
}

bool object_collide_ray(float *_dist, struct rico_object *object,
                        const struct ray *ray)
{
    return collide_ray_obb(_dist, ray, &object->bbox, &object->xform.matrix,
                           &object->xform.matrix_inverse);
}

bool object_collide_ray_type(struct pack *pack, struct rico_object **_object,
                             float *_dist, enum rico_obj_type type,
                             const struct ray *ray)
{
    bool collided = false;
    float distance;
    *_dist = Z_FAR; // Track closest object

    struct rico_object *obj = 0;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type != type)
            continue;

        collided = collide_ray_obb(&distance, ray, &obj->bbox,
                                   &obj->xform.matrix,
                                   &obj->xform.matrix_inverse);

        // If closest so far, save info
        if (collided && distance < *_dist)
        {
            // Record object handle and distance
            *_object = obj;
            *_dist = distance;
        }
    }

    return collided;
}

internal void object_interact_light_switch(struct rico_object *obj)
{
    UNUSED(obj);
}

void object_interact(struct rico_object *obj)
{
    if (state_is_edit())
    {
        object_select_toggle(obj);
    }
    else if (!state_is_paused())
    {
        switch (obj->type)
        {
        case OBJ_LIGHT_SWITCH:
            object_interact_light_switch(obj);
            break;
        default:
            break;
        }
    }
}

internal void object_update_static(struct rico_object *obj)
{
    UNUSED(obj);
}

void object_update(struct rico_object *obj)
{
    switch (obj->type)
    {
    case OBJ_STATIC:
        object_update_static(obj);
        break;
    default:
        break;
    }
}

void object_render_setup(enum rico_obj_type type,
                         const struct program_pbr *prog,
                         const struct camera *camera)
{
    if (type == OBJ_STATIC)
    {
        glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glUseProgram(prog->prog_id);

    // Model transform
    struct mat4 proj_matrix = camera->proj_matrix;
    struct mat4 view_matrix = camera->view_matrix;

    // TODO: Get the light out of here!!! It should't be updating its position
    //       in the render function, argh!
    u32 ticks = SDL_GetTicks();
    float light_pos_x = cosf((float)ticks / 2000.0f) * 3.0f;
    float light_pos_z = sinf((float)ticks / 2000.0f) * 3.0f;

    struct light_point light;
    light.pos = VEC3(light_pos_x, 3.0f, light_pos_z);
    light.color = VEC3(1.0f, 0.9f, 0.6f);
    light.intensity = 10.0f;
    //light.ambient = VEC3(0.10f, 0.09f, 0.11f);
    light.kc = 1.0f;
    light.kl = 0.05f;
    light.kq = 0.001f;

    if (type == OBJ_STRING_SCREEN)
    {
        // TODO: Create dedicated shader for OBJ_STRING_SCREEN instead of using
        //       all of these hacks to disable scaling, projection, lighting...
        proj_matrix = MAT4_IDENT;
        view_matrix = MAT4_IDENT;

        light.pos = VEC3_ZERO;
        light.color = VEC3_ONE;
        light.intensity = 10.0f;

        light.kc = 1.0f;
        light.kl = 0.0f;
        light.kq = 0.0f;
    }

    glUniformMatrix4fv(prog->projection, 1, GL_TRUE, proj_matrix.a);
    glUniformMatrix4fv(prog->view, 1, GL_TRUE, view_matrix.a);
    glUniform3fv(prog->camera.pos, 1, (const GLfloat *)&camera->position);

    // Material textures
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    glUniform1i(prog->material.tex0, 0);
    glUniform1i(prog->material.tex1, 1);

    // Lighting
    glUniform3fv(prog->light.pos, 1, (const GLfloat *)&light.pos);
    glUniform3fv(prog->light.color, 1, (const GLfloat *)&light.color);
    glUniform1f(prog->light.intensity, light.intensity);

    glUniform2f(prog->scale_uv, 1.0f, 1.0f);
}

void object_render_type(struct pack *pack, enum rico_obj_type type,
                        const struct program_pbr *prog,
                        const struct camera *camera)
{
    object_render_setup(type, prog, camera);

    //struct rico_pool *pool = chunk->pools[RICO_HND_OBJECT];
    //struct rico_object *obj = pool_first(pool);
    struct rico_object *obj = 0;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type != type)
            continue;

#if RICO_DEBUG_OBJECT
        printf("[ obj][rndr] name=%s\n", object_name(obj));
#endif

        // Set object-specific uniform values

        // UV-coord scale
        // HACK: This only works when object is uniformly scaled on X/Y
        //       plane.
        // TODO: UV scaling in general only works when object is uniformly
        //       scaled. Maybe I should only allow textured objects to be
        //       uniformly scaled?
        if (!(type == OBJ_STRING_WORLD || type == OBJ_STRING_SCREEN))
        {
            glUniform2f(prog->scale_uv, obj->xform.scale.x,
                        obj->xform.scale.y);
        }

        // Model matrix
        glUniformMatrix4fv(prog->model, 1, GL_TRUE, obj->xform.matrix.a);

        // Bind material
        struct pack *mat_pack = pack;
        u32 mat_id = object_material(obj);
        if (mat_id)
        {
            if (type == OBJ_STRING_SCREEN || type == OBJ_STRING_WORLD)
            {
                mat_pack = pack_default;
            }
        }
        else
        {
            mat_pack = pack_default;
            if (type == OBJ_STRING_SCREEN || type == OBJ_STRING_WORLD)
            {
                mat_id = FONT_DEFAULT_MATERIAL;
            }
            else
            {
                mat_id = MATERIAL_DEFAULT;
            }
        }
        material_bind(mat_pack, mat_id);

        // Render
        u32 mesh_id = object_mesh(obj);
        if (mesh_id)
        {
            mesh_render(pack, mesh_id);
        }
        else
        {
            mesh_render(pack_default, MESH_DEFAULT_BBOX);
        }

        // Clean up
        material_unbind(mat_pack, mat_id);
    }
    glUseProgram(0);

    // Render bounding boxes
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type != type)
            continue;

        if (state_is_edit(state_get()))
        {
            prim_draw_bbox(&obj->bbox, &obj->xform);
        }
    }
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
        u32 mesh_id = object_mesh(object);
        u32 material_id = object_material(object);
        struct rico_mesh *mesh = (mesh_id)
            ? pack_lookup(pack_active, mesh_id)
            : NULL;
        struct rico_material *material = (material_id)
            ? pack_lookup(pack_active, material_id)
            : NULL;

        const char *mesh_str = "---";
        u32 mesh_verts = 0;
        const char *material_str = "---";
        u32 mat_tex0_id = 0;
        const char *mat_tex0_str = "---";
        u32 mat_tex1_id = 0;
        const char *mat_tex1_str = "---";

        if (mesh)
        {
            mesh_str = mesh_name(mesh);
            mesh_verts = mesh->vertex_count;
        }
        if (material)
        {
            material_str = material_name(material);

            if (material->tex_id[0])
            {
                struct rico_texture *tex =
                    pack_lookup(pack_active, material->tex_id[0]);
                mat_tex0_id = tex->id;
                mat_tex0_str = texture_name(tex);
            }
            if (material->tex_id[1])
            {
                struct rico_texture *tex =
                    pack_lookup(pack_active, material->tex_id[1]);
                mat_tex1_id = tex->id;
                mat_tex1_str = texture_name(tex);
            }
        }

        len = snprintf(
            buf, sizeof(buf),
            "\n" \
            "Object [%u|%u] %s\n" \
            "  Type  %s\n"        \
            "  Trans %f %f %f\n"  \
            "  Rot   %f %f %f\n"  \
            "  Scale %f %f %f\n"  \
            "  BBox  %f %f %f\n"  \
            "\n" \
            "Mesh [%u|%u] %s\n"   \
            "  Verts %u\n"        \
            "\n" \
            "Material [%u|%u] %s\n" \
            "  Tex0 [%u|%u] %s\n"   \
            "  Tex1 [%u|%u] %s\n",
            ID_PACK(object->id), ID_BLOB(object->id),
            object_name(object),
            rico_obj_type_string[object->type],
            object->xform.trans.x, object->xform.trans.y, object->xform.trans.z,
            object->xform.rot.x,   object->xform.rot.y,   object->xform.rot.z,
            object->xform.scale.x, object->xform.scale.y, object->xform.scale.z,
            object->bbox.p.x, object->bbox.p.y, object->bbox.p.z,
            ID_PACK(mesh_id), ID_BLOB(mesh_id), mesh_str,
            mesh_verts,
            ID_PACK(material_id), ID_BLOB(material_id), material_str,
            ID_PACK(mat_tex0_id), ID_BLOB(mat_tex0_id), mat_tex0_str,
            ID_PACK(mat_tex1_id), ID_BLOB(mat_tex1_id), mat_tex1_str
        );
    }
    else
    {
        len = snprintf(buf, sizeof(buf), "No object selected");
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
