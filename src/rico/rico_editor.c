#define RIC_WIDGET_BOX_OFFSET 1.0f
#define RIC_WIDGET_BOX_RADIUS 0.1f

struct widget
{
    enum ric_widget action;
    struct ric_aabb aabb;
};
static struct widget widgets[RIC_WIDGET_COUNT];
static struct widget *widget;

static pkid selected_obj_id;

static struct program_pbr *global_prog_pbr;
static struct program_shadow_texture *global_prog_shadow_texture;
static struct program_shadow_cubemap *global_prog_shadow_cubemap;
static struct program_primitive *global_prog_primitive;
static struct program_text *global_prog_text;

static void edit_init()
{
    const float offset = RIC_WIDGET_BOX_OFFSET;
    const float radius = RIC_WIDGET_BOX_RADIUS;

    widgets[0].aabb.c = VEC3(offset + radius, 0.0f, 0.0f);
    widgets[0].aabb.e = VEC3_1(radius);
    widgets[0].action = RIC_WIDGET_TRANSLATE_X;

    widgets[1].aabb.c = VEC3(0.0f, offset + radius, 0.0f);
    widgets[1].aabb.e = VEC3_1(radius);
    widgets[1].action = RIC_WIDGET_TRANSLATE_Y;

    widgets[2].aabb.c = VEC3(0.0f, 0.0f, offset + radius);
    widgets[2].aabb.e = VEC3_1(radius);
    widgets[2].action = RIC_WIDGET_TRANSLATE_Z;
}
static void edit_free()
{
#if 0
    // TODO: What are chunks used for now? How/when do we need to free them?
    free(pack_frame);
    free(pack_transient);
    free(pack_default);

    // TODO: Free chunks/pools if still using any
    //chunk_free(chunk_active);
    //chunk_free(chunk_transient);

    //TODO: Free all programs
    free_program_pbr(&global_prog_pbr);
    free_program_primitive(&global_prog_primitive);
    free_program_text(&global_prog_text);
#endif
}
static void edit_object_create()
{
    if (!selected_obj_id)
        return;

    // TODO: Prompt user for object name
    const char *name = "new_obj";

    // Create new object and select it
    pkid new_id = ric_load_object(PKID_PACK(selected_obj_id), 0, 0, name);
    edit_object_select(new_id, false);
}
static void edit_aabb_reset_all()
{
    u32 start_id = PKID_PACK(selected_obj_id);
    object_aabb_recalculate_all(start_id);

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
        struct ric_object *obj = ric_pack_lookup(selected_obj_id);
        obj->selected = false;
    }

    // Select requested object
    if (id && (force || id != selected_obj_id))
    {
        struct ric_object *obj = ric_pack_lookup(id);
        obj->selected = true;
        selected_obj_id = id;
    }
    else
    {
        selected_obj_id = 0;
    }

    edit_print_object();
}
static void edit_pack_next()
{
    u32 start_id = PKID_PACK(selected_obj_id);
    u32 pack_id = 0;

    pkid id;
    for (;;)
    {
        if (global_packs[pack_id])
        {
            id = ric_pack_first_type(pack_id, RIC_ASSET_OBJECT);
            if (id) break;
        }
        pack_id = (pack_id + 1) % MAX_PACKS;
        if (pack_id == start_id)
        {
            return;
        }
    }

    edit_object_select(id, false);
}
static void edit_object_next()
{
    pkid next_id = 0;
    if (selected_obj_id)
    {
        next_id = ric_pack_next_loop(selected_obj_id);
    }
    edit_object_select(next_id, false);
}
static void edit_object_prev()
{
    pkid prev_id = 0;
    if (selected_obj_id)
    {
        prev_id = ric_pack_prev_loop(selected_obj_id);
    }
    edit_object_select(prev_id, false);
}
static void edit_print_object()
{
    struct ric_object *obj = NULL;
    if (selected_obj_id)
    {
        obj = ric_pack_lookup(selected_obj_id);
    }
    object_print(obj);
}
static void edit_translate(struct ric_camera *camera, const struct vec3 *offset)
{
    if (!selected_obj_id)
        return;

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);

    if (v3_equals(offset, &VEC3_ZERO))
    {
        if (camera->locked)
        {
            camera->pos = VEC3_ZERO;
        }
        ric_object_trans_set(obj, &VEC3_ZERO);
    }
    else
    {
        if (camera->locked)
        {
            camera_translate_world(camera, offset);
        }
        ric_object_trans(obj, offset);
    }

    object_print(obj);
}
static void edit_rotate(const struct quat *offset)
{
    if (!selected_obj_id)
        return;

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
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

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
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

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
    pkid next_material_id = 0;

    u32 pack_start = PKID_PACK(obj->material_id);
    u32 pack_id = pack_start + 1;
    while (pack_id != pack_start)
    {
        if (pack_id != RIC_PACK_ID_TRANSIENT &&
            pack_id != RIC_PACK_ID_FRAME &&
            global_packs[pack_id])
        {
            next_material_id = ric_pack_last_type(pack_id, RIC_ASSET_MATERIAL);
            if (next_material_id)
                break;
        }
        pack_id++;
        pack_id = pack_id % MAX_PACKS;
    }

    if (next_material_id)
    {
        ric_object_material_set(obj, next_material_id);
        object_print(obj);
    }
}
static void edit_material_next()
{
    if (!selected_obj_id)
        return;

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
    pkid next_material_id = (obj->material_id)
        ? ric_pack_next_type_loop(obj->material_id, RIC_ASSET_MATERIAL)
        : ric_pack_first_type(RIC_PACK_ID_DEFAULT, RIC_ASSET_MATERIAL);
    struct ric_material *next_material = ric_pack_lookup(next_material_id);
    if (next_material)
    {
        ric_object_material_set(obj, next_material->uid.pkid);
        object_print(obj);
    }
}
static void edit_material_prev()
{
    if (!selected_obj_id)
        return;

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
    pkid prev_material_id = (obj->material_id)
        ? ric_pack_prev_type_loop(obj->material_id, RIC_ASSET_MATERIAL)
        : ric_pack_last_type(RIC_PACK_ID_DEFAULT, RIC_ASSET_MATERIAL);
    struct ric_material *prev_material = ric_pack_lookup(prev_material_id);
    if (prev_material)
    {
        ric_object_material_set(obj, prev_material->uid.pkid);
        object_print(obj);
    }
}
static void edit_mesh_next_pack()
{
    if (!selected_obj_id)
        return;

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
    pkid next_mesh_id = 0;

    u32 pack_start = PKID_PACK(obj->mesh_id);
    u32 pack_id = pack_start + 1;
    while (pack_id != pack_start)
    {
        if (pack_id != RIC_PACK_ID_TRANSIENT &&
            pack_id != RIC_PACK_ID_FRAME &&
            global_packs[pack_id])
        {
            next_mesh_id = ric_pack_last_type(pack_id, RIC_ASSET_MESH);
            if (next_mesh_id)
                break;
        }
        pack_id++;
        pack_id = pack_id % MAX_PACKS;
    }

    if (next_mesh_id)
    {
        ric_object_mesh_set(obj, next_mesh_id);
        obj->selected = true;
        object_print(obj);
    }
}
static void edit_mesh_next()
{
    if (!selected_obj_id)
        return;

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
    pkid next_mesh_id = (obj->mesh_id)
        ? ric_pack_next_type_loop(obj->mesh_id, RIC_ASSET_MESH)
        : ric_pack_first_type(RIC_PACK_ID_DEFAULT, RIC_ASSET_MESH);
    if (next_mesh_id)
    {
        ric_object_mesh_set(obj, next_mesh_id);
        obj->selected = true;
        object_print(obj);
    }
}
static void edit_mesh_prev()
{
    if (!selected_obj_id)
        return;

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
    pkid prev_mesh_id = (obj->mesh_id)
        ? ric_pack_prev_type_loop(obj->mesh_id, RIC_ASSET_MESH)
        : ric_pack_last_type(RIC_PACK_ID_DEFAULT, RIC_ASSET_MESH);
    if (prev_mesh_id)
    {
        ric_object_mesh_set(obj, prev_mesh_id);
        obj->selected = true;
        object_print(obj);
    }
}
static void edit_aabb_reset()
{
    if (!selected_obj_id)
        return;

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
    object_aabb_recalculate(obj);

    obj->selected = true;
    object_print(obj);
}
static void edit_duplicate()
{
    if (!selected_obj_id)
        return;

    // TODO: Prompt user for name
    const char *name = "Duplicate";

    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
    struct ric_object *new_obj = object_copy(PKID_PACK(selected_obj_id), obj,
                                              name);
    edit_object_select(new_obj->uid.pkid, false);
}
static void edit_delete()
{
    if (!selected_obj_id)
        return;

    pkid prev_obj = ric_pack_prev_loop(selected_obj_id);
    pack_delete(selected_obj_id);
    selected_obj_id = 0;
    edit_object_select(prev_obj, false);
}
static struct widget *widget_test()
{
    struct widget *widget = NULL;

    if (selected_obj_id)
    {
        struct ric_object *obj = ric_pack_lookup(selected_obj_id);
        struct ray cam_ray = { 0 };
        camera_fwd_ray(&cam_player, &cam_ray);

        float dist_min = 9999.0;
        for (u32 i = 0; i < ARRAY_COUNT(widgets); ++i)
        {
            struct ric_aabb aabb = widgets[i].aabb;
            v3_add(&aabb.c, &obj->xform.position);

            float dist;
            bool collide = collide_ray_aabb(&dist, &cam_ray, &aabb);
            if (collide && dist < dist_min)
            {
                dist_min = dist;
                widget = &widgets[i];
            }
        }

        if (widget)
        {
            string_free_slot(RIC_STRING_SLOT_WIDGET);
            ric_load_string(RIC_PACK_ID_TRANSIENT, RIC_STRING_SLOT_WIDGET,
                             SCREEN_X(0), SCREEN_Y(FONT_HEIGHT),
                             COLOR_DARK_CYAN, 0, 0,
                             ric_widget_string[widget->action]);
        }
    }

    return widget;
}
extern bool ric_mouse_raycast(pkid *_obj_id, float *_dist)
{
    // Camera forward ray v. scene
    struct ray cam_ray = { 0 };
    camera_fwd_ray(&cam_player, &cam_ray);
    return object_collide_ray_type(_obj_id, _dist, &cam_ray);
}
static void edit_mouse_pressed()
{
    // Hit test widgets
    widget = widget_test();
    if (widget) return;

    // Select first object w/ ray pick
    pkid obj_collided_id = 0;
    ric_mouse_raycast(&obj_collided_id, 0);
    edit_object_select(obj_collided_id, false);
}
static void edit_mouse_move()
{
    if (!selected_obj_id) return;
    if (!widget) return;

    struct ray cam_ray;
    camera_fwd_ray(&cam_player, &cam_ray);

    bool collide = false;
    struct ric_object *obj = ric_pack_lookup(selected_obj_id);
    struct vec3 trans = obj->xform.position;

    struct vec3 normal = cam_player.pos;
    v3_sub(&normal, &obj->xform.position);

    if (widget->action == RIC_WIDGET_TRANSLATE_X)
    {
        normal.x = 0.0f;
        v3_normalize(&normal);

        //prim_draw_plane(&obj->xform.position, &normal, &MAT4_IDENT,
        //                &COLOR_RED_HIGHLIGHT);


        struct vec3 contact = { 0 };
        collide = collide_ray_plane(&contact, &cam_ray,
                                    &PLANE(obj->xform.position, normal));
        if (collide)
        {
            trans.x = contact.x - RIC_WIDGET_BOX_OFFSET - RIC_WIDGET_BOX_RADIUS;
            trans.x -= (float)fmod(trans.x, global_trans_delta);
        }
    }
    else if (widget->action == RIC_WIDGET_TRANSLATE_Y)
    {
        normal.y = 0.0f;
        v3_normalize(&normal);

        //prim_draw_plane(&obj->xform.position, &normal, &MAT4_IDENT,
        //                &COLOR_GREEN_HIGHLIGHT);

        struct vec3 contact = { 0 };
        collide = collide_ray_plane(&contact, &cam_ray,
                                    &PLANE(obj->xform.position, normal));
        if (collide)
        {
            trans.y = contact.y - RIC_WIDGET_BOX_OFFSET - RIC_WIDGET_BOX_RADIUS;
            trans.y -= (float)fmod(trans.y, global_trans_delta);
        }
    }
    else if (widget->action == RIC_WIDGET_TRANSLATE_Z)
    {
        normal.z = 0.0f;
        v3_normalize(&normal);

        //prim_draw_plane(&obj->xform.position, &normal, &MAT4_IDENT,
        //                &COLOR_BLUE_HIGHLIGHT);

        struct vec3 contact = { 0 };
        collide = collide_ray_plane(&contact, &cam_ray,
                                    &PLANE(obj->xform.position, normal));
        if (collide)
        {
            trans.z = contact.z - RIC_WIDGET_BOX_OFFSET - RIC_WIDGET_BOX_RADIUS;
            trans.z -= (float)fmod(trans.z, global_trans_delta);
        }
    }

    if (collide)
    {
        ric_object_trans_set(obj, &trans);
        object_print(obj);
    }
}
static void edit_mouse_released()
{
    widget = NULL;
    string_free_slot(RIC_STRING_SLOT_WIDGET);
}
static void edit_render()
{
    //--------------------------------------------------------------------------
    // Origin axes
    //--------------------------------------------------------------------------
    ric_prim_draw_line(&VEC3_ZERO, &VEC3_X, &COLOR_RED);
    ric_prim_draw_line(&VEC3_ZERO, &VEC3_Y, &COLOR_GREEN);
    ric_prim_draw_line(&VEC3_ZERO, &VEC3_Z, &COLOR_BLUE);

    //--------------------------------------------------------------------------
    // Selected object widget
    //--------------------------------------------------------------------------
    glClear(GL_DEPTH_BUFFER_BIT);

    if (selected_obj_id)
    {
        struct ric_object *obj = ric_pack_lookup(selected_obj_id);

        struct mat4 xform = MAT4_IDENT;
        mat4_translate(&xform, &obj->xform.position);

        ric_prim_draw_line_xform(&VEC3_ZERO, &VEC3_X, &COLOR_RED, &xform);
        ric_prim_draw_line_xform(&VEC3_ZERO, &VEC3_Y, &COLOR_GREEN, &xform);
        ric_prim_draw_line_xform(&VEC3_ZERO, &VEC3_Z, &COLOR_BLUE, &xform);

        ric_prim_draw_aabb_xform(&widgets[0].aabb, &COLOR_RED, &xform);
        ric_prim_draw_aabb_xform(&widgets[1].aabb, &COLOR_GREEN, &xform);
        ric_prim_draw_aabb_xform(&widgets[2].aabb, &COLOR_BLUE, &xform);
    }

#if 0
    struct quad flag1 = {
        VEC3(2.0f, 2.0f, 0.0f),
        VEC3(3.0f, 2.0f, 0.0f),
        VEC3(2.0f, 3.0f, 0.0f),
        VEC3(3.0f, 3.0f, 0.0f)
    };
    prim_draw_quad(&flag1, &MAT4_IDENT, &COLOR_CYAN_HIGHLIGHT);

    struct vec3 n = VEC3(0.1f, 0.5f, 0.8f);
    struct vec3 center = VEC3(2.0f, 2.0f, 2.0f);
    prim_draw_plane(&center, &n, &COLOR_MAGENTA_HIGHLIGHT);
#endif

    //--------------------------------------------------------------------------
    // UI
    //--------------------------------------------------------------------------
    glClear(GL_DEPTH_BUFFER_BIT);

    struct program_text *prog = global_prog_text;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    RICO_ASSERT(prog->program.gl_id);
    glUseProgram(prog->program.gl_id);

    // Projection matrix
    glUniformMatrix4fv(prog->locations.vert.proj, 1, GL_TRUE,
                       cam_player.ortho_matrix.a);
    glUniformMatrix4fv(prog->locations.vert.view, 1, GL_TRUE, MAT4_IDENT.a);

    glUniform4fv(prog->locations.frag.color, 1, &COLOR_WHITE.r);
    glUniform1i(prog->locations.frag.grayscale, false);
    // Font texture
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    glUniform1i(prog->locations.frag.tex0, 0);

    string_render_all(prog->locations.vert.model);

    glUseProgram(0);
}