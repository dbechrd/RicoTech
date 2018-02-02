#define WIDGET_BOX_OFFSET 1.0f;
#define WIDGET_BOX_RADIUS 0.1f;

const char *widget_action_string[] = {
    WIDGET_ACTIONS(GEN_STRING)
};

struct widget
{
    struct bbox bbox;
    enum widget_action action;
};
struct widget widgets[3];
struct widget *widget;

global u32 selected_obj_id;

struct program_pbr *prog_pbr;
struct program_primitive *prog_prim;
struct program_text *prog_text;

void glref_init()
{
    const float radius = WIDGET_BOX_RADIUS;
    const float offset = WIDGET_BOX_OFFSET;

    widgets[0].bbox.min = VEC3(offset - radius, -radius, -radius);
    widgets[0].bbox.max = VEC3(offset + radius, radius, radius);
    widgets[0].action = WIDGET_TRANSLATE_X;

    widgets[1].bbox.min = VEC3(-radius, offset - radius, -radius);
    widgets[1].bbox.max = VEC3(radius, offset + radius, radius);
    widgets[1].action = WIDGET_TRANSLATE_Y;

    widgets[2].bbox.min = VEC3(-radius, -radius, offset - radius);
    widgets[2].bbox.max = VEC3(radius, radius, offset + radius);
    widgets[2].action = WIDGET_TRANSLATE_Z;
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

struct widget *widget_test()
{
    struct widget *widget = NULL;

    if (selected_obj_id)
    {
        struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
        struct ray cam_ray = { 0 };
        camera_fwd_ray(&cam_ray, &cam_player);

        struct mat4 xform = MAT4_IDENT;
        mat4_translate(&xform, &obj->xform.trans);

        float dist_min = 9999.0;
        for (u32 i = 0; i < array_count(widgets); ++i)
        {
            float dist;
            bool collide = collide_ray_bbox(&dist, &cam_ray, &widgets[i].bbox,
                                            &xform);
            if (collide && dist < dist_min)
            {
                dist_min = dist;
                widget = &widgets[i];
            }
        }

        if (widget)
        {
            string_free_slot(STR_SLOT_WIDGET);
            load_string(
                packs[PACK_TRANSIENT], "WIDGET_STATE", STR_SLOT_WIDGET,
                -(FONT_WIDTH * strlen(widget_action_string[widget->action])),
                FONT_HEIGHT * 5, COLOR_DARK_CYAN, 0, NULL,
                widget_action_string[widget->action]
            );
        }
    }

    return widget;
}

struct rico_object *mouse_first_obj()
{
    struct rico_object *obj_collided = { 0 };
    float dist;

    // Camera forward ray v. scene
    struct ray cam_ray = { 0 };
    camera_fwd_ray(&cam_ray, &cam_player);
    object_collide_ray_type(pack_active, &obj_collided, &dist, &cam_ray);

    return obj_collided;
}

void edit_mouse_pressed()
{
    // Hit test widgets
    widget = widget_test();
    if (widget) return;

    // Select first object w/ ray pick
    select_obj(mouse_first_obj(), false);
}

void edit_mouse_move(r32 dx, r32 dy)
{
    UNUSED(dx);
    UNUSED(dy);

    if (!selected_obj_id)
        return;
    if (!widget)
        return;

    struct ray cam_ray;
    camera_fwd_ray(&cam_ray, &cam_player);

    bool collide = false;
    struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);
    struct vec3 trans = obj->xform.trans;

    struct vec3 normal = cam_player.pos;
    v3_sub(&normal, &obj->xform.trans);

    if (widget->action == WIDGET_TRANSLATE_X)
    {
        normal.x = 0.0f;
        v3_normalize(&normal);

        prim_draw_plane(&obj->xform.trans, &normal, &obj->xform.matrix,
                        &COLOR_RED_HIGHLIGHT);

        struct vec3 contact = { 0 };
        collide = collide_ray_plane(&contact, &cam_ray, &obj->xform.trans,
                                    &normal);
        if (collide) {
            trans.x = contact.x - WIDGET_BOX_OFFSET;
            trans.x -= (float)fmod(trans.x, trans_delta);
        }
    }
    else if (widget->action == WIDGET_TRANSLATE_Y)
    {
        normal.y = 0.0f;
        v3_normalize(&normal);
        
        prim_draw_plane(&obj->xform.trans, &normal, &obj->xform.matrix,
                        &COLOR_GREEN_HIGHLIGHT);

        struct vec3 contact = { 0 };
        collide = collide_ray_plane(&contact, &cam_ray, &obj->xform.trans,
                                    &normal);
        if (collide) {
            trans.y = contact.y - WIDGET_BOX_OFFSET;
            trans.y -= (float)fmod(trans.y, trans_delta);
        }
    }
    else if (widget->action == WIDGET_TRANSLATE_Z)
    {
        normal.z = 0.0f;
        v3_normalize(&normal);

        prim_draw_plane(&obj->xform.trans, &normal, &obj->xform.matrix,
                        &COLOR_BLUE_HIGHLIGHT);

        struct vec3 contact = { 0 };
        collide = collide_ray_plane(&contact, &cam_ray, &obj->xform.trans,
                                    &normal);
        if (collide) {
            trans.z = contact.z - WIDGET_BOX_OFFSET;
            trans.z -= (float)fmod(trans.z, trans_delta);
        }
    }

    if (collide)
    {
        object_trans_set(obj, &trans);
        object_print(obj);
    }
}

void edit_mouse_released()
{
    widget = NULL;
    string_free_slot(STR_SLOT_WIDGET);
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
    // Origin axes
    //--------------------------------------------------------------------------
    prim_draw_line(VEC3_ZERO, VEC3_X, &MAT4_IDENT, COLOR_RED);
    prim_draw_line(VEC3_ZERO, VEC3_Y, &MAT4_IDENT, COLOR_GREEN);
    prim_draw_line(VEC3_ZERO, VEC3_Z, &MAT4_IDENT, COLOR_BLUE);

    //--------------------------------------------------------------------------
    // Selected object widget
    //--------------------------------------------------------------------------
    if (selected_obj_id)
    {
        struct rico_object *obj = pack_lookup(pack_active, selected_obj_id);

        struct mat4 xform = MAT4_IDENT;
        mat4_translate(&xform, &obj->xform.trans);

        prim_draw_line(VEC3_ZERO, VEC3_X, &xform, COLOR_RED);
        prim_draw_line(VEC3_ZERO, VEC3_Y, &xform, COLOR_GREEN);
        prim_draw_line(VEC3_ZERO, VEC3_Z, &xform, COLOR_BLUE);
        prim_draw_bbox(&widgets[0].bbox, &xform, &COLOR_RED);
        prim_draw_bbox(&widgets[1].bbox, &xform, &COLOR_GREEN);
        prim_draw_bbox(&widgets[2].bbox, &xform, &COLOR_BLUE);
    }

#if 0
    struct quad test = {
        VEC3(2.0f, 2.0f, 0.0f),
        VEC3(3.0f, 2.0f, 0.0f),
        VEC3(2.0f, 3.0f, 0.0f),
        VEC3(3.0f, 3.0f, 0.0f)
    };
    prim_draw_quad(&test, &MAT4_IDENT, &COLOR_CYAN_HIGHLIGHT);

    struct vec3 n = VEC3(0.1f, 0.5f, 0.8f);
    struct vec3 p = VEC3(2.0f, 2.0f, 2.0f);
    prim_draw_plane(&p, &n, &COLOR_MAGENTA_HIGHLIGHT);
#endif

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
