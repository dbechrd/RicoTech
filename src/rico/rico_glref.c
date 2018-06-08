#define WIDGET_BOX_OFFSET 1.0f;
#define WIDGET_BOX_RADIUS 0.1f;

static const char *widget_action_string[] = { WIDGET_ACTIONS(GEN_STRING) };

struct widget
{
    struct RICO_bbox bbox;
    enum widget_action action;
};
static struct widget widgets[3];
static struct widget *widget;

static pkid selected_obj_id;

static struct pbr_program *prog_pbr;
static struct prim_program *prog_prim;
static struct text_program *prog_text;

static void editor_init()
{
    const float offset = WIDGET_BOX_OFFSET;
    const float radius = WIDGET_BOX_RADIUS;

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
static void edit_object_create()
{
    // TODO: Prompt user for object name
    const char *name = "new_obj";

    // HACK: Use first user-defined type
    // Create new object and select it
    pkid obj_id = RICO_load_object(RICO_pack_active, RICO_OBJECT_TYPE_COUNT, 0,
                                   name);
    edit_object_select(obj_id, false);
}
static void edit_bbox_reset_all()
{
    object_bbox_recalculate_all(RICO_pack_active);

    // Reselect current object
    if (selected_obj_id)
    {
        edit_object_select(selected_obj_id, true);
    }
}
static void edit_object_select(pkid id, bool force)
{
    // NULL already selected
    if (!force && !id && !selected_obj_id)
        return;

    // Deselect current object
    if (selected_obj_id)
    {
        struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
        object_deselect(obj);
    }

    // Select requested object
    if (id && (force || id != selected_obj_id))
    {
        struct RICO_object *obj = RICO_pack_lookup(id);
        object_select(obj);
        selected_obj_id = id;
    }
    else
    {
        selected_obj_id = 0;
    }

    edit_print_object();
}
static pkid find_first_selectable_object(u32 pack_id)
{
    struct pack *pack = packs[pack_id];
    RICO_ASSERT(pack);

    pkid id = RICO_pack_first(pack_id);
    while (id)
    {
        struct RICO_object *obj = RICO_pack_lookup(id);
        if (obj->uid.type == RICO_HND_OBJECT && object_selectable(obj))
        {
            break;
        }
        id = RICO_pack_next(id);
    }
    return id;
}
static void edit_pack_next()
{
    u32 start_id = PKID_PACK(selected_obj_id);
    u32 pack_id = (start_id + 1) % MAX_PACKS;

    pkid id;
    while (pack_id != start_id)
    {
        if (packs[pack_id])
        {
            id = find_first_selectable_object(pack_id);
            if (id)
                break;
        }
        pack_id = (pack_id + 1) % MAX_PACKS;
    }

    edit_object_select(id, false);
}
static void edit_object_next()
{
    pkid next_obj = (selected_obj_id)
        ? RICO_pack_next_loop(selected_obj_id)
        : RICO_pack_first_type(PACK_DEFAULT, RICO_HND_OBJECT);
    edit_object_select(next_obj, false);
}
static void edit_object_prev()
{
    pkid prev_obj = (selected_obj_id)
        ? RICO_pack_prev_loop(selected_obj_id)
        : RICO_pack_last_type(PACK_DEFAULT, RICO_HND_OBJECT);
    edit_object_select(prev_obj, false);
}
static void edit_print_object()
{
    struct RICO_object *obj = NULL;
    if (selected_obj_id)
        obj = RICO_pack_lookup(selected_obj_id);
    object_print(obj);
}
static void edit_translate(struct RICO_camera *camera, const struct vec3 *offset)
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);

    // HACK: There's probably a better way to do this check
    bool has_bbox = !v3_equals(&obj->bbox.min, &obj->bbox.max);

    if (v3_equals(offset, &VEC3_ZERO))
    {
        if (camera->locked && has_bbox)
        {
            camera->pos = VEC3_ZERO;
        }
        RICO_object_trans_set(obj, &VEC3_ZERO);
    }
    else
    {
        if (camera->locked && has_bbox)
        {
            camera_translate_world(camera, offset);
        }
        RICO_object_trans(obj, offset);
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
static void edit_material_next_pack()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    pkid next_material_id = 0;

    u32 pack_start = PKID_PACK(obj->material_id);
    u32 pack_id = pack_start + 1;
    while (pack_id != pack_start)
    {
        if (pack_id != PACK_TRANSIENT &&
            pack_id != PACK_FRAME &&
            packs[pack_id])
        {
            next_material_id = RICO_pack_last_type(pack_id, RICO_HND_MATERIAL);
            if (next_material_id)
                break;
        }
        pack_id++;
        pack_id = pack_id % MAX_PACKS;
    }

    if (next_material_id)
    {
        obj->material_id = next_material_id;
        object_print(obj);
    }
}
static void edit_material_next()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    pkid next_material_id = (obj->material_id)
        ? RICO_pack_next_loop(obj->material_id)
        : RICO_pack_first_type(PACK_DEFAULT, RICO_HND_MATERIAL);
    struct RICO_material *next_material = RICO_pack_lookup(next_material_id);
    if (next_material)
    {
        obj->material_id = next_material->uid.pkid;
        object_print(obj);
    }
}
static void edit_material_prev()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    pkid prev_material_id = (obj->material_id)
        ? RICO_pack_prev_loop(obj->material_id)
        : RICO_pack_last_type(PACK_DEFAULT, RICO_HND_MATERIAL);
    struct RICO_material *prev_material = RICO_pack_lookup(prev_material_id);
    if (prev_material)
    {
        obj->material_id = prev_material->uid.pkid;
        object_print(obj);
    }
}
static void edit_mesh_next_pack()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    pkid next_mesh_id = 0;

    u32 pack_start = PKID_PACK(obj->mesh_id);
    u32 pack_id = pack_start + 1;
    while (pack_id != pack_start)
    {
        if (pack_id != PACK_TRANSIENT &&
            pack_id != PACK_FRAME &&
            packs[pack_id])
        {
            next_mesh_id = RICO_pack_last_type(pack_id, RICO_HND_MESH);
            if (next_mesh_id)
                break;
        }
        pack_id++;
        pack_id = pack_id % MAX_PACKS;
    }

    if (next_mesh_id)
    {
        RICO_object_mesh_set(obj, next_mesh_id);
        object_select(obj);
        object_print(obj);
    }
}
static void edit_mesh_next()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    pkid next_mesh_id = (obj->mesh_id)
        ? RICO_pack_next_loop(obj->mesh_id)
        : RICO_pack_first_type(PACK_DEFAULT, RICO_HND_MESH);
    if (next_mesh_id)
    {
        RICO_object_mesh_set(obj, next_mesh_id);
        object_select(obj);
        object_print(obj);
    }
}
static void edit_mesh_prev()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    pkid prev_mesh_id = (obj->mesh_id)
        ? RICO_pack_prev_loop(obj->mesh_id)
        : RICO_pack_last_type(PACK_DEFAULT, RICO_HND_MESH);
    if (prev_mesh_id)
    {
        RICO_object_mesh_set(obj, prev_mesh_id);
        object_select(obj);
        object_print(obj);
    }
}
static void edit_bbox_reset()
{
    if (!selected_obj_id)
        return;

    struct RICO_object *obj = RICO_pack_lookup(selected_obj_id);
    object_bbox_recalculate(obj);

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
    struct RICO_object *new_obj = object_copy(PKID_PACK(selected_obj_id), obj,
                                              name);
    edit_object_select(new_obj->uid.pkid, false);
}
static void edit_delete()
{
    if (!selected_obj_id)
        return;

    pkid prev_obj = RICO_pack_prev_loop(selected_obj_id);
    pack_delete(selected_obj_id);
    selected_obj_id = 0;
    edit_object_select(prev_obj, false);
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
            RICO_load_string(PACK_TRANSIENT, STR_SLOT_WIDGET,
                             SCREEN_X(0), SCREEN_Y(FONT_HEIGHT),
                             COLOR_DARK_CYAN, 0, 0,
                             widget_action_string[widget->action]);
        }
    }

    return widget;
}
extern bool RICO_mouse_raycast(pkid *_obj_id, float *_dist)
{
    // Camera forward ray v. scene
    struct ray cam_ray = { 0 };
    camera_fwd_ray(&cam_ray, &cam_player);
    return object_collide_ray_type(_obj_id, _dist, &cam_ray);
}
static void edit_mouse_pressed()
{
    // Hit test widgets
    widget = widget_test();
    if (widget) return;

    // Select first object w/ ray pick
    pkid obj_collided_id = 0;
    RICO_mouse_raycast(&obj_collided_id, 0);
    edit_object_select(obj_collided_id, false);
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
        RICO_object_trans_set(obj, &trans);
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
    RICO_prim_draw_line(&VEC3_ZERO, &VEC3_X, &COLOR_RED);
    RICO_prim_draw_line(&VEC3_ZERO, &VEC3_Y, &COLOR_GREEN);
    RICO_prim_draw_line(&VEC3_ZERO, &VEC3_Z, &COLOR_BLUE);

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

        RICO_prim_draw_line_xform(&VEC3_ZERO, &VEC3_X, &COLOR_RED, &xform);
        RICO_prim_draw_line_xform(&VEC3_ZERO, &VEC3_Y, &COLOR_GREEN, &xform);
        RICO_prim_draw_line_xform(&VEC3_ZERO, &VEC3_Z, &COLOR_BLUE, &xform);

        RICO_prim_draw_bbox_xform(&widgets[0].bbox, &COLOR_RED, &xform);
        RICO_prim_draw_bbox_xform(&widgets[1].bbox, &COLOR_GREEN, &xform);
        RICO_prim_draw_bbox_xform(&widgets[2].bbox, &COLOR_BLUE, &xform);
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

    for (u32 i = PACK_COUNT; i < ARRAY_COUNT(packs); ++i)
    {
        if (packs[i])
            object_render_ui(packs[i], &cam_player);
    }
    object_render_ui(packs[PACK_TRANSIENT], &cam_player);
    object_render_ui(packs[PACK_FRAME], &cam_player);
}
static void free_glref()
{
#if 0
    // TODO: What are chunks used for now? How/when do we need to free them?
    free(pack_frame);
    free(pack_transient);
    free(RICO_pack_active);
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
