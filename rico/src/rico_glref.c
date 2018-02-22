#define WIDGET_BOX_OFFSET 1.0f;
#define WIDGET_BOX_RADIUS 0.1f;

static const char *widget_action_string[] = { WIDGET_ACTIONS(GEN_STRING) };

struct widget
{
    struct RICO_bbox RICO_bbox;
    enum widget_action action;
};
static struct widget widgets[3];
static struct widget *widget;

static pkid selected_obj_id;

static struct program_pbr *prog_pbr;
static struct program_primitive *prog_prim;
static struct program_text *prog_text;

static void editor_init()
{
    const float offset = WIDGET_BOX_OFFSET;
    const float radius = WIDGET_BOX_RADIUS;

    widgets[0].RICO_bbox.min = VEC3(offset - radius, -radius, -radius);
    widgets[0].RICO_bbox.max = VEC3(offset + radius, radius, radius);
    widgets[0].action = WIDGET_TRANSLATE_X;

    widgets[1].RICO_bbox.min = VEC3(-radius, offset - radius, -radius);
    widgets[1].RICO_bbox.max = VEC3(radius, offset + radius, radius);
    widgets[1].action = WIDGET_TRANSLATE_Y;

    widgets[2].RICO_bbox.min = VEC3(-radius, -radius, offset - radius);
    widgets[2].RICO_bbox.max = VEC3(radius, radius, offset + radius);
    widgets[2].action = WIDGET_TRANSLATE_Z;
}
static void edit_object_create(struct pack *pack)
{
    // TODO: Prompt user for object name
    const char *name = "new_obj";

    // Create new object and select it
    pkid obj_pkid = RICO_load_object(pack, RICO_OBJECT_TYPE_START, 0, name);
    struct RICO_object *obj = RICO_pack_lookup(obj_pkid);
    edit_object_select(obj, false);
}
static void edit_bbox_reset_all()
{
    object_bbox_recalculate_all(pack_active);

    // Reselect current object
    if (selected_obj_id)
    {
        struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
        edit_object_select(obj, true);
    }
}
static void edit_object_select(struct RICO_object *rico, bool force)
{
    // NULL already selected
    if (!force && !rico && !selected_obj_id)
        return;

    // Deselect current object
    if (selected_obj_id)
    {
        struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
        object_deselect(obj);
    }

    // Select requested object
    if (rico && (force || rico->uid.pkid != selected_obj_id))
    {
        object_select(rico);
        selected_obj_id = rico->uid.pkid;
    }
    else
    {
        selected_obj_id = 0;
    }

    edit_print_object();
}
static void edit_object_next()
{
    edit_object_select(pack_next(selected_obj_id, RICO_HND_OBJECT), false);
}
static void edit_object_prev()
{
    edit_object_select(pack_prev(selected_obj_id, RICO_HND_OBJECT), false);
}
static void edit_print_object()
{
    struct RICO_object *obj = NULL;
    if (selected_obj_id)
        obj = RICO_pack_lookup(selected_obj_id);
    object_print(obj);
}
static void edit_translate(struct camera *camera, const struct vec3 *offset)
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);

    // HACK: There's probably a better way to do this check
    bool has_bbox = !v3_equals(&obj->RICO_bbox.min, &obj->RICO_bbox.max);

    if (v3_equals(offset, &VEC3_ZERO))
    {
        if (camera->locked && has_bbox)
        {
            camera->pos = VEC3_ZERO;
        }
        object_trans_set(obj, &VEC3_ZERO);
    }
    else
    {
        if (camera->locked && has_bbox)
        {
            camera_translate_world(camera, offset);
        }
        object_trans(obj, offset);
    }

    object_print(obj);
}
static void edit_rotate(const struct quat *offset)
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    if (quat_equals(offset, &QUAT_IDENT))
    {
        object_rot_set(obj, &QUAT_IDENT);
    }
    else
    {
        object_rot(obj, offset);
    }

    object_print(obj);
}
static void edit_scale(const struct vec3 *offset)
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
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
static void edit_material_next()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    struct RICO_material *next_material = pack_next(obj->material_pkid,
                                                    RICO_HND_MATERIAL);
    if (next_material)
    {
        obj->material_pkid = next_material->uid.pkid;
        object_print(obj);
    }
}
static void edit_material_prev()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    struct RICO_material *prev_material = pack_prev(obj->material_pkid,
                                                    RICO_HND_MATERIAL);
    if (prev_material)
    {
        obj->material_pkid = prev_material->uid.pkid;
        object_print(obj);
    }
}
static void edit_mesh_next()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    struct RICO_mesh *next_mesh = pack_next(obj->mesh_pkid,
                                            RICO_HND_MESH);
    if (next_mesh)
    {
        obj->mesh_pkid = next_mesh->uid.pkid;
        obj->RICO_bbox = next_mesh->RICO_bbox;
        object_select(obj);
        object_print(obj);
    }
}
static void edit_mesh_prev()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    struct RICO_mesh *prev_mesh = pack_prev(obj->mesh_pkid,
                                            RICO_HND_MESH);
    if (prev_mesh)
    {
        obj->mesh_pkid = prev_mesh->uid.pkid;
        obj->RICO_bbox = prev_mesh->RICO_bbox;
        object_select(obj);
        object_print(obj);
    }
}
static void edit_bbox_reset()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    struct RICO_mesh *mesh = RICO_pack_lookup(obj->mesh_pkid);
    obj->RICO_bbox = mesh->RICO_bbox;

    object_select(obj);
    object_print(obj);
}
static void edit_duplicate()
{
    if (!selected_obj_id)
        return;

    // TODO: Prompt user for name
    const char *name = "Duplicate";

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    struct RICO_object *new_obj = object_copy(RICO_packs[PKID_PACK(selected_obj_id)],
                                              obj, name);
    edit_object_select(new_obj, false);
}
static void edit_delete()
{
    if (!selected_obj_id)
        return;

    pack_delete(selected_obj_id);
    edit_object_prev();
}
static struct widget *widget_test()
{
    struct widget *widget = NULL;

    if (selected_obj_id)
    {
        struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
        struct ray cam_ray = { 0 };
        camera_fwd_ray(&cam_ray, &cam_player);

        struct mat4 xform = MAT4_IDENT;
        mat4_translate(&xform, &obj->xform.position);

        float dist_min = 9999.0;
        for (u32 i = 0; i < ARRAY_COUNT(widgets); ++i)
        {
            float dist;
            bool collide = collide_ray_bbox(&dist, &cam_ray, &widgets[i].RICO_bbox,
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
            RICO_load_string(RICO_packs[PACK_TRANSIENT], STR_SLOT_WIDGET,
                             SCREEN_X(0), SCREEN_Y(FONT_HEIGHT),
                             COLOR_DARK_CYAN, 0, 0,
                             widget_action_string[widget->action]);
        }
    }

    return widget;
}
static struct RICO_object *mouse_first_obj()
{
    struct RICO_object *obj_collided = { 0 };
    float dist;

    // Camera forward ray v. scene
    struct ray cam_ray = { 0 };
    camera_fwd_ray(&cam_ray, &cam_player);
    object_collide_ray_type(pack_active, &obj_collided, &dist, &cam_ray);

    return obj_collided;
}
static void edit_mouse_pressed()
{
    // Hit test widgets
    widget = widget_test();
    if (widget) return;

    // Select first object w/ ray pick
    edit_object_select(mouse_first_obj(), false);
}
static void edit_mouse_move()
{
    if (!selected_obj_id) return;
    if (!widget || widget->action == WIDGET_NONE) return;

    struct ray cam_ray;
    camera_fwd_ray(&cam_ray, &cam_player);

    bool collide = false;
    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    struct vec3 trans = obj->xform.position;

    struct vec3 normal = cam_player.pos;
    v3_sub(&normal, &obj->xform.position);

    if (widget->action == WIDGET_TRANSLATE_X)
    {
        normal.x = 0.0f;
        v3_normalize(&normal);

        //prim_draw_plane(&obj->xform.position, &normal, &MAT4_IDENT,
        //                &COLOR_RED_HIGHLIGHT);

        struct vec3 contact = { 0 };
        collide = collide_ray_plane(&contact, &cam_ray,
                                    &obj->xform.position,
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

        //prim_draw_plane(&obj->xform.position, &normal, &MAT4_IDENT,
        //                &COLOR_GREEN_HIGHLIGHT);

        struct vec3 contact = { 0 };
        collide = collide_ray_plane(&contact, &cam_ray,
                                    &obj->xform.position,
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

        //prim_draw_plane(&obj->xform.position, &normal, &MAT4_IDENT,
        //                &COLOR_BLUE_HIGHLIGHT);

        struct vec3 contact = { 0 };
        collide = collide_ray_plane(&contact, &cam_ray,
                                    &obj->xform.position,
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
static void edit_mouse_released()
{
    widget = NULL;
    string_free_slot(STR_SLOT_WIDGET);
}
static void edit_render()
{
    //--------------------------------------------------------------------------
    // Origin axes
    //--------------------------------------------------------------------------
    prim_draw_line(&VEC3_ZERO, &VEC3_X, &MAT4_IDENT, COLOR_RED);
    prim_draw_line(&VEC3_ZERO, &VEC3_Y, &MAT4_IDENT, COLOR_GREEN);
    prim_draw_line(&VEC3_ZERO, &VEC3_Z, &MAT4_IDENT, COLOR_BLUE);

    //--------------------------------------------------------------------------
    // Selected object widget
    //--------------------------------------------------------------------------
    glClear(GL_DEPTH_BUFFER_BIT);

    if (selected_obj_id)
    {
        struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);

        struct mat4 xform = MAT4_IDENT;
        mat4_translate(&xform, &obj->xform.position);
        
        //struct vec3 *bbox = &obj->bbox.max;
        //prim_draw_line(&VEC3_ZERO, &VEC3(bbox->x, 0.0f, 0.0f), &xform, COLOR_RED);
        //prim_draw_line(&VEC3_ZERO, &VEC3(0.0f, bbox->y, 0.0f), &xform, COLOR_GREEN);
        //prim_draw_line(&VEC3_ZERO, &VEC3(0.0f, 0.0f, bbox->z), &xform, COLOR_BLUE);

        prim_draw_line(&VEC3_ZERO, &VEC3_X, &xform, COLOR_RED);
        prim_draw_line(&VEC3_ZERO, &VEC3_Y, &xform, COLOR_GREEN);
        prim_draw_line(&VEC3_ZERO, &VEC3_Z, &xform, COLOR_BLUE);
        
        prim_draw_bbox(&widgets[0].RICO_bbox, &xform, &COLOR_RED);
        prim_draw_bbox(&widgets[1].RICO_bbox, &xform, &COLOR_GREEN);
        prim_draw_bbox(&widgets[2].RICO_bbox, &xform, &COLOR_BLUE);
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
    glClear(GL_DEPTH_BUFFER_BIT);

    for (u32 i = PACK_COUNT; i < ARRAY_COUNT(RICO_packs); ++i)
    {
        if (RICO_packs[i])
            object_render_ui(RICO_packs[i]);
    }
    object_render_ui(RICO_packs[PACK_TRANSIENT]);
    object_render_ui(RICO_packs[PACK_FRAME]);
}
static void free_glref()
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
