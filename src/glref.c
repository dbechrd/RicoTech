global u32 selected_obj_id;

struct program_pbr *prog_pbr;
struct program_primitive *prog_prim;
struct program_text *prog_text;

global struct bbox axis_bbox;

global struct rico_transform x_axis_transform;
global struct rico_transform y_axis_transform;
global struct rico_transform z_axis_transform;

void init_glref()
{
    // TODO: Find somewhere to put debug/editor draw code that makes sense
    //--------------------------------------------------------------------------
    // Create axis label bboxes
    //--------------------------------------------------------------------------
    // TODO: Just use prim_cube!
    struct rico_mesh *mesh = pack_lookup(packs[PACK_DEFAULT],
                                         MESH_DEFAULT_CUBE);
    axis_bbox = mesh->bbox;

    const float trans = 0.0f;

    // X-axis label
    x_axis_transform.scale = VEC3(1.0f, 0.01f, 0.01f);
    x_axis_transform.trans = VEC3(trans, 0.0f, 0.0f);

    // Y-axis label
    y_axis_transform.scale = VEC3(0.01f, 1.0f, 0.01f);
    y_axis_transform.trans = VEC3(0.0f, trans, 0.0f);

    // Z-axis label
    z_axis_transform.scale = VEC3(0.01f, 0.01f, 1.0f);
    z_axis_transform.trans = VEC3(0.0f, 0.0f, trans);
}

void create_obj(struct pack *pack)
{
    // TODO: Prompt user for object name
    const char *name = "new_obj";

    // TODO: Allow properties to be added dynamically
    // Create new object and select it
    struct obj_property props[2] = { 0 };
    props[0].type = PROP_MESH_ID;
    props[0].mesh_id = 0;
    props[1].type = PROP_MATERIAL_ID;
    props[1].material_id = 0;
    u32 new_obj_id = load_object(pack, name, OBJ_STATIC, array_count(props),
                                 props, NULL);
    struct rico_object *obj = pack_lookup(pack, new_obj_id);
    select_obj(obj, false);
}

void recalculate_all_bbox()
{
    object_bbox_recalculate_all(pack_active);

    // Reselect current object
    if (selected_obj_id)
    {
        struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
        select_obj(obj, true);
    }
}

void select_obj(struct rico_object *object, bool force)
{
    // NULL already selected
    if (!force && !object && !selected_obj_id)
        return;
    
    // Deselect current object
    if (selected_obj_id)
    {
        struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
        object_deselect(obj);
    }

    // Select requested object
    if (object && (force || object->id != selected_obj_id))
    {
        object_select(object);
        selected_obj_id = object->id;
    }
    else
    {
        selected_obj_id = 0;
    }

    selected_print();
}

void select_next_obj()
{
    select_obj(pack_next(pack_active, selected_obj_id, RICO_HND_OBJECT), false);
}

void select_prev_obj()
{
    select_obj(pack_prev(pack_active, selected_obj_id, RICO_HND_OBJECT), false);
}

void selected_print()
{
    struct rico_object *obj = NULL;
    if (selected_obj_id)
        obj = pack_lookup(pack_active, selected_obj_id);
    object_print(obj);
}

void selected_translate(struct camera *camera, const struct vec3 *offset)
{
    if (!selected_obj_id)
        return;

    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    bool selectable = object_selectable(obj);

    if (v3_equals(offset, &VEC3_ZERO))
    {
        if (camera->locked && selectable)
        {
            camera->pos = VEC3_ZERO;
        }
        object_trans_set(obj, &VEC3_ZERO);
    }
    else
    {
        if (camera->locked && selectable)
        {
            camera_translate_world(camera, offset);
        }
        object_trans(obj, offset);
    }

    object_print(obj);
}

void selected_rotate(const struct vec3 *offset)
{
    if (!selected_obj_id)
        return;

    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    if (v3_equals(offset, &VEC3_ZERO))
    {
        object_rot_set(obj, &VEC3_ZERO);
    }
    else
    {
        object_rot(obj, offset);
    }

    object_print(obj);
}

void selected_scale(const struct vec3 *offset)
{
    if (!selected_obj_id)
        return;

    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    if (v3_equals(offset, &VEC3_ZERO))
    {
        object_scale_set(obj, &VEC3_ONE);
    }
    else
    {
        object_scale(obj, offset);
    }

    object_print(obj);
}

void selected_material_next()
{
    if (!selected_obj_id)
        return;

    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    struct obj_property *mat_prop = object_prop(obj, PROP_MATERIAL_ID);
    if (mat_prop)
    {
        struct rico_material *next_material =
            pack_next(pack_active, mat_prop->material_id,
                      RICO_HND_MATERIAL);
        if (next_material)
        {
            mat_prop->material_id = next_material->id;
            object_print(obj);
        }
    }
}

void selected_material_prev()
{
    if (!selected_obj_id)
        return;

    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    struct obj_property *mat_prop = object_prop(obj, PROP_MATERIAL_ID);
    if (mat_prop)
    {
        struct rico_material *prev_material =
            pack_prev(pack_active, mat_prop->material_id,
                      RICO_HND_MATERIAL);
        if (prev_material)
        {
            mat_prop->material_id = prev_material->id;
            object_print(obj);
        }
    }
}

void selected_mesh_next()
{
    if (!selected_obj_id)
        return;

    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    struct obj_property *mesh_prop = object_prop(obj, PROP_MESH_ID);
    if (mesh_prop)
    {
        struct rico_mesh *next_mesh =
            pack_next(pack_active, mesh_prop->mesh_id,
                      RICO_HND_MESH);
        if (next_mesh)
        {
            mesh_prop->mesh_id = next_mesh->id;
            obj->bbox = next_mesh->bbox;
            object_select(obj);
            object_print(obj);
        }
    }
}

void selected_mesh_prev()
{
    if (!selected_obj_id)
        return;

    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    struct obj_property *mesh_prop = object_prop(obj, PROP_MESH_ID);
    if (mesh_prop)
    {
        struct rico_mesh *prev_mesh =
            pack_prev(pack_active, mesh_prop->mesh_id,
                      RICO_HND_MESH);
        if (prev_mesh)
        {
            mesh_prop->mesh_id = prev_mesh->id;
            obj->bbox = prev_mesh->bbox;
            object_select(obj);
            object_print(obj);
        }
    }
}

void selected_bbox_reset()
{
    if (!selected_obj_id)
        return;

    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    struct obj_property *mesh_prop = object_prop(obj, PROP_MESH_ID);
    if (!mesh_prop)
        return;

    struct rico_mesh *mesh = pack_lookup(pack_active, mesh_prop->mesh_id);
    obj->bbox = mesh->bbox;

    object_select(obj);
    object_print(obj);
}

void selected_duplicate()
{
    if (!selected_obj_id)
        return;

    // TODO: Prompt user for name
    const char *name = "Duplicate";

    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    struct rico_object *new_obj = object_copy(pack_active, obj, name);
    select_obj(new_obj, false);
}

void selected_delete()
{
    if (!selected_obj_id)
        return;

    pack_delete(pack_active, selected_obj_id, RICO_HND_OBJECT);
    select_prev_obj();
}

void glref_update()
{
    //--------------------------------------------------------------------------
    // Update uniforms
    //--------------------------------------------------------------------------
    glUseProgram(prog_pbr->prog_id);
    glUniform1f(prog_pbr->time, (r32)SIM_SEC);
    glUseProgram(0);
}

void glref_render(struct camera *camera)
{
    //--------------------------------------------------------------------------
    // Render objects
    //--------------------------------------------------------------------------
    for (u32 i = PACK_COUNT; i < array_count(packs); ++i)
    {
        if (packs[i])
            object_render(packs[i], camera);
    }
    object_render(packs[PACK_TRANSIENT], camera);
    object_render(packs[PACK_FRAME], camera);

    //--------------------------------------------------------------------------
    // Axes labels (bboxes)
    //--------------------------------------------------------------------------
    //prim_draw_bbox_color(&axis_bbox, &x_axis_transform, &COLOR_RED);
    //prim_draw_bbox_color(&axis_bbox, &y_axis_transform, &COLOR_GREEN);
    //prim_draw_bbox_color(&axis_bbox, &z_axis_transform, &COLOR_BLUE);

    //--------------------------------------------------------------------------
    // UI
    //--------------------------------------------------------------------------
    for (u32 i = PACK_COUNT; i < array_count(packs); ++i)
    {
        if (packs[i])
            object_render_ui(packs[i]);
    }
    object_render_ui(packs[PACK_TRANSIENT]);
    object_render_ui(packs[PACK_FRAME]);
}
void free_glref()
{
#if 0
    // TODO: What are chunks used for now? How/when do we need to free them?
    free(pack_frame);
    free(pack_transient);
    free(pack_active);
    free(pack_default);

    // TODO: Free chunks/pools if still using any
    //chunk_free(chunk_active);
    //chunk_free(chunk_transient);

    //TODO: Free all programs
    free_program_pbr(&prog_pbr);
    free_program_primitive(&prog_prim);
    free_program_text(&prog_text);
#endif
}
