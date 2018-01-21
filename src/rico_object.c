const char *rico_obj_type_string[] = {
    RICO_OBJ_TYPES(GEN_STRING)
};

global const char *object_name(struct rico_object *obj)
{
    RICO_ASSERT(obj->name_offset);
    return (char *)((u8 *)obj + obj->name_offset);
}
global struct obj_property *object_props(struct rico_object *obj)
{
    if (!obj->prop_count)
        return NULL;
    return (struct obj_property *)((u8 *)obj + obj->props_offset);
}
global struct obj_property *object_prop(struct rico_object *obj,
                                        enum obj_prop_type type)
{
    struct obj_property *props = object_props(obj);
    for (u32 i = 0; i < obj->prop_count; ++i)
    {
        if (props[i].type == type)
            return &props[i];
    }
    return NULL;
}

global void object_delete(struct pack *pack, struct rico_object *obj)
{
    struct obj_property *props = object_props(obj);
    for (u32 i = 0; i < obj->prop_count; ++i)
    {
        if (props[i].type == PROP_MESH_ID)
        {
            pack_delete(pack, props[i].mesh_id, RICO_HND_MESH);
        }
        else if (props[i].type == PROP_MATERIAL_ID)
        {
            pack_delete(pack, props[i].material_id, RICO_HND_MATERIAL);
        }
    }
}

global struct rico_object *object_copy(struct pack *pack,
                                       struct rico_object *other,
                                       const char *name)
{
    // Create new object with same mesh / texture
    u32 new_obj_id = load_object(pack, name, other->type, other->prop_count,
                                 object_props(other), &other->bbox);
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
    struct obj_property *prop;
    struct rico_mesh *mesh;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type != OBJ_STATIC)
            continue;

        prop = object_prop(obj, PROP_MESH_ID);
        if (prop)
        {
            mesh = pack_lookup(pack, prop->mesh_id);
        }
        else
        {
            mesh = pack_lookup(pack_default, MESH_DEFAULT_CUBE);
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
                             float *_dist, const struct ray *ray)
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
        if (obj->type == OBJ_TERRAIN)
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

// HACK: I want to make a light switch!
static bool lights_on = true;
internal void object_interact_light_switch(struct rico_object *obj)
{
    UNUSED(obj);
    lights_on = !lights_on;
}

// HACK: I want to make an audio switch!
static bool audio_play = true;
internal void object_interact_audio_switch(struct rico_object *obj)
{
    UNUSED(obj);
    audio_play = !audio_play;
    if (audio_play)
        alSourcePlay(audio_source);
    else
        alSourceStop(audio_source);
}

typedef void (*prop_interactor)(struct rico_object *obj);
prop_interactor interactors[PROP_COUNT] = {
    0,                            // PROP_MESH_ID
    0,                            // PROP_MATERIAL_ID
    0,                            // PROP_LIGHT_DIR
    0,                            // PROP_LIGHT_POINT
    0,                            // PROP_LIGHT_SPOT
    object_interact_light_switch, // PROP_LIGHT_SWITCH
    object_interact_audio_switch  // PROP_AUDIO_SWITCH
};

void object_interact(struct rico_object *obj)
{
    if (state_is_edit())
    {
        object_select_toggle(obj);
    }
    else if (!state_is_paused())
    {
        struct obj_property *props = object_props(obj);
        for (u32 i = 0; i < obj->prop_count; ++i)
        {
            prop_interactor interactor = interactors[props[i].type];
            if (interactor) interactor(obj);
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

void object_render(struct pack *pack, const struct camera *camera)
{
    struct program_pbr *prog = prog_pbr;
    glUseProgram(prog->prog_id);

    // TODO: Get the light out of here!!! It should't be updating its position
    //       in the render function, argh!
    struct vec3 light_pos = { 0 };

    light_pos = cam_player.pos;
    v3_add(&light_pos, &VEC3(0.0f, -0.5f, 0.0f));

    struct light_point light;
    light.position = light_pos;
    light.color = VEC3(1.0f, 0.8f, 0.4f);
    light.intensity = (lights_on) ? 10.0f : 0.0f;
    //light.ambient = VEC3(0.10f, 0.09f, 0.11f);
    light.kc = 1.0f;
    light.kl = 0.05f;
    light.kq = 0.001f;

    glUniform3fv(prog->camera.pos, 1, (const GLfloat *)&camera->pos);

    // Material textures
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    glUniform1i(prog->material.tex0, 0);
    glUniform1i(prog->material.tex1, 1);
    glUniform1i(prog->material.tex2, 2);

    // Lighting
    glUniform3fv(prog->light.pos, 1, (const GLfloat *)&light.position);
    glUniform3fv(prog->light.color, 1, (const GLfloat *)&light.color);
    glUniform1f(prog->light.intensity, light.intensity);

    glUniform2f(prog->scale_uv, 1.0f, 1.0f);

    struct rico_object *obj = 0;
    enum rico_obj_type prev_type = OBJ_NULL;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type == OBJ_STRING_SCREEN)
            continue;

#if RICO_DEBUG_OBJECT
        printf("[ obj][rndr] name=%s\n", object_name(obj));
#endif

        if (obj->type != prev_type)
        {
            glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);

            // Model transform
            struct mat4 proj_matrix;
            struct mat4 view_matrix;

            proj_matrix = camera->proj_matrix;
            view_matrix = camera->view_matrix;

            glUniformMatrix4fv(prog->proj, 1, GL_TRUE, proj_matrix.a);
            glUniformMatrix4fv(prog->view, 1, GL_TRUE, view_matrix.a);

            prev_type = obj->type;
        }

        // Set object-specific uniform values

        // UV-coord scale
        // HACK: This only works when object is uniformly scaled on X/Y
        //       plane.
        // TODO: UV scaling in general only works when object is uniformly
        //       scaled. Maybe I should only allow textured objects to be
        //       uniformly scaled?
        if (obj->type == OBJ_STRING_WORLD)
        {
            // TODO: Why can't I just assume obj->xform.scale is always 1,1,1?
            glUniform2f(prog->scale_uv, 1.0f, 1.0f);
        }
        else
        {
            glUniform2f(prog->scale_uv, obj->xform.scale.x, obj->xform.scale.y);
        }

        // Model matrix
        glUniformMatrix4fv(prog->model, 1, GL_TRUE, obj->xform.matrix.a);

        // Bind material
        struct pack *mat_pack = pack;
        u32 mat_id = 0;

        struct obj_property *mat_prop = object_prop(obj, PROP_MATERIAL_ID);
        if (mat_prop && mat_prop->material_id)
        {
            if (obj->type == OBJ_STRING_WORLD)
            {
                mat_pack = pack_default;
            }
            mat_id = mat_prop->material_id;
        }
        else
        {
            mat_pack = pack_default;
            if (obj->type == OBJ_STRING_WORLD)
            {
                mat_id = FONT_DEFAULT_TEXTURE;
            }
            else
            {
                mat_id = MATERIAL_DEFAULT;
            }
        }
        material_bind(mat_pack, mat_id);

        // Render
        struct obj_property *mesh_prop = object_prop(obj, PROP_MESH_ID);
        if (mesh_prop && mesh_prop->mesh_id)
        {
            mesh_render(pack, mesh_prop->mesh_id, prog->type);
        }
        else
        {
            mesh_render(pack_default, MESH_DEFAULT_CUBE, prog->type);
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

        if (state_is_edit(state_get()))
        {
            prim_draw_bbox(&obj->bbox, &obj->xform);
        }
    }
}

void object_render_ui(struct pack *pack)
{
    struct program_text *prog = prog_text;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(prog->prog_id);

    // Font texture
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    glUniform1i(prog->tex, 0);

    struct rico_object *obj = 0;
    for (u32 index = 1; index < pack->blobs_used; ++index)
    {
        if (pack->index[index].type != RICO_HND_OBJECT)
            continue;

        obj = pack_read(pack, index);
        if (obj->type != OBJ_STRING_SCREEN)
            continue;

#if RICO_DEBUG_OBJECT
        printf("[ obj][rndr] name=%s\n", object_name(obj));
#endif

        // Model matrix
        glUniformMatrix4fv(prog->model, 1, GL_TRUE, obj->xform.matrix.a);

        // Bind texture
        struct pack *tex_pack = pack;
        u32 tex_id = 0;

        struct obj_property *tex_prop = object_prop(obj, PROP_TEXTURE_ID);
        if (tex_prop && tex_prop->texture_id)
        {
            // TODO: Let UI use textures outside of default pack?
            tex_pack = pack_default;
            tex_id = tex_prop->texture_id;
        }
        else
        {
            tex_pack = pack_default;
            tex_id = FONT_DEFAULT_TEXTURE;
        }
        texture_bind(tex_pack, tex_id, GL_TEXTURE0);

        // Render
        struct obj_property *mesh_prop = object_prop(obj, PROP_MESH_ID);
        if (mesh_prop && mesh_prop->mesh_id)
        {
            mesh_render(pack, mesh_prop->mesh_id, prog->type);
        }
        else
        {
            // Screen strings should always have a mesh set.
            RICO_ASSERT(0);
        }

        // Clean up
        texture_unbind(tex_pack, tex_id, GL_TEXTURE0);
    }
    glUseProgram(0);
}

void object_print(struct rico_object *obj)
{
    string_free_slot(STR_SLOT_SELECTED_OBJ);

    int len;
    char buf[BFG_MAXSTRING + 1] = { 0 };

    if (obj)
    {
        struct obj_property *mesh_prop = object_prop(obj, PROP_MESH_ID);
        struct obj_property *material_prop = object_prop(obj, PROP_MATERIAL_ID);

        u32 mesh_id = (mesh_prop) ? mesh_prop->mesh_id : 0;
        u32 material_id = (material_prop) ? material_prop->material_id : 0;
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
        u32 mat_tex2_id = 0;
        const char *mat_tex2_str = "---";

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
            if (material->tex_id[2])
            {
                struct rico_texture *tex =
                    pack_lookup(pack_active, material->tex_id[2]);
                mat_tex2_id = tex->id;
                mat_tex2_str = texture_name(tex);
            }
        }

        len = snprintf(
            buf, sizeof(buf),
            "\n"                    \
            "Object [%u|%u] %s\n"   \
            "  Type  %s\n"          \
            "  Trans %f %f %f\n"    \
            "  Rot   %f %f %f\n"    \
            "  Scale %f %f %f\n"    \
            "  BBox  %f %f %f\n"    \
            "        %f %f %f\n"    \
            "\n"                    \
            "Mesh [%u|%u] %s\n"     \
            "  Verts %u\n"          \
            "\n"                    \
            "Material [%u|%u] %s\n" \
            "  Diff [%u|%u] %s\n"   \
            "  Spec [%u|%u] %s\n"   \
            "  Emis [%u|%u] %s\n",
            ID_PACK(obj->id), ID_BLOB(obj->id),
            object_name(obj),
            rico_obj_type_string[obj->type],
            obj->xform.trans.x, obj->xform.trans.y, obj->xform.trans.z,
            obj->xform.rot.x,   obj->xform.rot.y,   obj->xform.rot.z,
            obj->xform.scale.x, obj->xform.scale.y, obj->xform.scale.z,
            obj->bbox.min.x, obj->bbox.min.y, obj->bbox.min.z,
            obj->bbox.max.x, obj->bbox.max.y, obj->bbox.max.z,
            ID_PACK(mesh_id), ID_BLOB(mesh_id), mesh_str,
            mesh_verts,
            ID_PACK(material_id), ID_BLOB(material_id), material_str,
            ID_PACK(mat_tex0_id), ID_BLOB(mat_tex0_id), mat_tex0_str,
            ID_PACK(mat_tex1_id), ID_BLOB(mat_tex1_id), mat_tex1_str,
            ID_PACK(mat_tex2_id), ID_BLOB(mat_tex2_id), mat_tex2_str
        );
    }
    else
    {
        len = snprintf(buf, sizeof(buf), "No object selected");
    }

    string_truncate(buf, sizeof(buf), len);
    load_string(pack_transient, rico_string_slot_string[STR_SLOT_SELECTED_OBJ],
                STR_SLOT_SELECTED_OBJ, 0, FONT_HEIGHT,
                COLOR_DARK_GRAY_HIGHLIGHT, 0, NULL, buf);
}
