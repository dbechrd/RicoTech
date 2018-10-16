#include "chet.h"
#include "rico.c"
#include "chet_pack.c"
#include "chet_pack_default.c"
#include "chet_pack_alpha.c"
#include "chet_pack_clash.c"

enum actions
{
    CHET_ACTION_TEST_SOUND = RIC_ACTION_COUNT,
    CHET_ACTION_TYPE_NEXT,
    CHET_ACTION_TYPE_PREV
};

enum audio_type
{
    AUDIO_WELCOME,
    AUDIO_THUNDER,
    AUDIO_BUTTON,
    AUDIO_VICTORY,
    AUDIO_COUNT
};

//-------------------------------------------------------------------

static struct game_panel panel_1;

static struct ric_audio_buffer audio_buffers[AUDIO_COUNT];
static struct ric_audio_source audio_sources[AUDIO_COUNT];

#define TOOLBAR_ICON(f) \
    f(TOOLBAR_CURSOR,     "Cursor")            \
    f(TOOLBAR_TRANSLATE,  "Translate")         \
    f(TOOLBAR_ROTATE,     "Rotate")            \
    f(TOOLBAR_SCALE,      "Scale")             \
    f(TOOLBAR_MESH,       "Mesh")              \
    f(TOOLBAR_TEXTURE,    "Texture")           \
    f(TOOLBAR_NEW,        "Create new object") \
    f(TOOLBAR_COPY,       "Copy selected")     \
    f(TOOLBAR_DELETE,     "Delete selected")   \
    f(TOOLBAR_UNDO,       "Undo")              \
    f(TOOLBAR_REDO,       "Redo")              \
    f(TOOLBAR_SAVE,       "Save pack")         \
    f(TOOLBAR_EXIT,       "Exit")

enum toolbar_icon
{
    TOOLBAR_ICON(ENUM)
    TOOLBAR_COUNT
};
//static const char *toolbar_icon_string[] = { TOOLBAR_ICON(ENUM_STRING) };
static const char *toolbar_icon_values[] = { TOOLBAR_ICON(ENUM_META) };

static struct ric_sprite toolbar_sprites[TOOLBAR_COUNT];
static struct ric_spritesheet toolbar_sheet;
static struct ui_tooltip toolbar_tips[TOOLBAR_COUNT];

// TODO: Remove hard-coded pack name enum; use strings / ids
enum {
    PACK_ALPHA = 1,
    PACK_CLASH = 2
};

static struct pack_info *pack_table;

//-------------------------------------------------------------------
// TODO: Scale by delta_time properly
#define GRAVITY VEC3(0.0f, -0.0098f, 0.0f)
// TODO: Elastic collision coef
static const float COEF_ELASTICITY = 0.15f;

void pack_build_all()
{
    pack_build_default(&pack_table, "packs/default.pak");
    pack_build_alpha(&pack_table, "packs/alpha.pak");
    pack_build_clash_of_cubes(&pack_table, "packs/clash.pak");
}

int pack_load_all()
{
	enum ric_error err;

    for (struct pack_info *pack = pack_table; pack != dlb_vec_end(pack_table); pack++)
    {
        PERF_START(pack_load);
        err = ric_pack_load(pack->path, &pack->id);
        if (err) return err;
        PERF_END_MSG(pack_load, "Loaded pack '%s'\n", pack->path);
    }

	return err;
}

void timmy_state_hacks(bool lights_on, bool audio_on)
{
    global_lighting_enabled = lights_on;
    if (audio_on)
    {
        ric_audio_unmute();
    }
    else
    {
        ric_audio_mute();
    }
}

void timmy_interact(struct timmy *timmy)
{
    timmy->lights_on = !timmy->lights_on;
    timmy->audio_on = !timmy->audio_on;
    timmy_state_hacks(timmy->lights_on, timmy->audio_on);
}

void game_button_interact(struct game_button *button)
{
    struct game_panel *panel = ric_pack_lookup(button->panel_id);
    DLB_ASSERT(button->index < ARRAY_COUNT(panel->buttons));

    pkid mat_on = PKID_GENERATE(3, 4);
    pkid mat_off = PKID_GENERATE(3, 5);

    // Toggle this button
    u32 mat = button->rico.material_id == mat_on ? mat_off : mat_on;
    ric_object_material_set(&button->rico, mat);

    // Toggle neighbor buttons
    pkid button_id;
    struct game_button *other;
    // UP
    if (button->index >= 3)
    {
        button_id = panel->buttons[button->index - 3];
        other = ric_pack_lookup(button_id);
        u32 mat = other->rico.material_id == mat_on ? mat_off : mat_on;
        ric_object_material_set(&other->rico, mat);
    }
    // DOWN
    if (button->index < 6)
    {
        button_id = panel->buttons[button->index + 3];
        other = ric_pack_lookup(button_id);
        u32 mat = other->rico.material_id == mat_on ? mat_off : mat_on;
        ric_object_material_set(&other->rico, mat);
    }
    // LEFT
    if (button->index % 3 > 0)
    {
        button_id = panel->buttons[button->index - 1];
        other = ric_pack_lookup(button_id);
        u32 mat = other->rico.material_id == mat_on ? mat_off : mat_on;
        ric_object_material_set(&other->rico, mat);
    }
    // RIGHT
    if (button->index % 3 < 2)
    {
        button_id = panel->buttons[button->index + 1];
        other = ric_pack_lookup(button_id);
        u32 mat = other->rico.material_id == mat_on ? mat_off : mat_on;
        ric_object_material_set(&other->rico, mat);
    }

    bool victory = true;
    for (u32 i = 0; i < ARRAY_COUNT(panel->buttons); i++)
    {
        struct game_button *button = ric_pack_lookup(panel->buttons[i]);
        if (button->rico.material_id == mat_on)
        {
            victory = false;
            break;
        }
    }

    ric_audio_source_play(&audio_sources[AUDIO_BUTTON]);
    if (victory)
    {
        ric_audio_source_play(&audio_sources[AUDIO_VICTORY]);
    }
}

static struct sphere rayviz_sphere;
void object_interact()
{
    rayviz_sphere.r = 0.0f;

    pkid obj_id = 0;
    float dist;
    bool collided = ric_mouse_raycast(&obj_id, &dist);
    if (!collided)
        return;

    float scale = 0.05f;

    struct ric_camera *camera = ric_get_camera_hack();
    struct vec3 pos = camera->pos;
    struct vec3 fwd;
    ric_camera_fwd(camera, &fwd);
    v3_add(&pos, v3_scalef(&fwd, dist));

    // TODO: Subtract (dot(dist, surface.normal) * scale) to place exactly on
    //       surface of mesh? This will place origin of new mesh on surface of
    //       existing mesh.. also need to somehow calculate and subtract
    //       distance from original to edge of new mesh and dot that with the
    //       ray to prevent it from being placed inside.
    rayviz_sphere.center = pos;
    rayviz_sphere.r = scale;

    if (dist > 10.0f)
        return;

    struct ric_object *obj = ric_pack_lookup(obj_id);
    if (!obj) return;

    switch (obj->type)
    {
        case OBJ_TIMMY:
            timmy_interact((struct timmy *)obj);
            break;
        case OBJ_GAME_BUTTON:
            game_button_interact((struct game_button *)obj);
            break;
    }
}

#if _DEBUG
#include <stdio.h>
DLB_ASSERT_HANDLER(handle_assert)
{
    printf("%s:%d ASSERT: %s", file, line, expr);
}
DLB_assert_handler_def *DLB_assert_handler = handle_assert;
#endif

bool ric_object_has_physics(pkid id)
{
    struct ric_object *obj = ric_pack_lookup(id);
    // TODO: Implement a proper physics flag
    return v3_length(&obj->aabb.e) > 0;
}

pkid ric_physics_next(pkid id)
{
    pkid next_id = id;
    do
    {
        next_id = ric_pack_next_loop(next_id);
    } while (next_id && next_id != id && !ric_object_has_physics(next_id));
    return id;
}

bool DEBUG_sphere_v_sphere(const struct sphere *a, const struct sphere *b,
                           struct manifold *manifold)
{
    struct vec3 D = a->center;
    v3_sub(&D, &b->center);
    float dist = v3_length(&D);
    float r_sum = a->r + b->r;
    bool collide = (dist < r_sum);

    if (collide)
    {
        float pen = r_sum - dist;
        struct vec3 normal = D;
        v3_normalize(&normal);
        struct vec3 position = normal;
        v3_scalef(&position, b->r - pen);
        v3_add(&position, &b->center);

        manifold->contacts[0].penetration = pen;
        manifold->contacts[0].normal = normal;
        manifold->contacts[0].position = position;
        manifold->contact_count = 1;
    }

    return collide;
}

void DEBUG_render_manifold(struct manifold *manifold)
{
    if (manifold->contact_count > 0)
    {
        for (u32 i = 0; i < manifold->contact_count; ++i)
        {
            struct sphere manifold_pos;
            manifold_pos.center = manifold->contacts[i].position;
            manifold_pos.r = 0.01f;
            ric_prim_draw_sphere(&manifold_pos, &COLOR_PINK);

            struct vec3 p0 = manifold->contacts[i].position;
            struct vec3 p1 = manifold->contacts[i].normal;
            v3_scalef(&p1, manifold->contacts[i].penetration);
            v3_add(&p1, &p0);
            ric_prim_draw_line(&p0, &p1, &COLOR_ORANGE);
        }
    }
}

void clash_detect()
{
    for (u32 i = 1; i < ARRAY_COUNT(global_packs); ++i)
    {
        if (!global_packs[i])
            continue;

        pkid a_id = ric_pack_first_type(global_packs[i]->id, RIC_ASSET_OBJECT);
        while (a_id)
        {
            struct ric_object *a_obj = ric_pack_lookup(a_id);
            a_obj->collide_sphere = false;
            a_obj->collide_aabb = false;
            a_obj->collide_obb = false;

            a_id = ric_pack_next_type(a_id, RIC_ASSET_OBJECT);
        }
    }
    for (u32 i = 1; i < ARRAY_COUNT(global_packs); ++i)
    {
        if (!global_packs[i])
            continue;

        pkid a_id = ric_pack_first_type(global_packs[i]->id, RIC_ASSET_OBJECT);
        while (a_id)
        {
            struct ric_object *a_obj = ric_pack_lookup(a_id);
            if (a_obj->type == OBJ_TERRAIN)
            {
                a_id = ric_pack_next_type(a_id, RIC_ASSET_OBJECT);
                continue;
            }

            for (u32 j = i; j < ARRAY_COUNT(global_packs); ++j)
            {
                if (!global_packs[j])
                    continue;

                pkid b_id;
                if (j == i)
                {
                    b_id = ric_pack_next_type(a_id, RIC_ASSET_OBJECT);
                }
                else
                {
                    b_id = ric_pack_first_type(global_packs[j]->id, RIC_ASSET_OBJECT);
                }

                while (b_id)
                {
                    struct ric_object *b_obj = ric_pack_lookup(b_id);
                    if (b_obj->type == OBJ_TERRAIN)
                    {
                        b_id = ric_pack_next_type(a_id, RIC_ASSET_OBJECT);
                        continue;
                    }

                    struct manifold manifold = { 0 };
                    manifold.body_a = a_obj;
                    manifold.body_b = b_obj;
                    bool collide_sphere = DEBUG_sphere_v_sphere(
                        &a_obj->sphere, &b_obj->sphere, &manifold);

                    DEBUG_render_manifold(&manifold);
                    if (manifold.contact_count > 0)
                    {
                        struct vec3 resolve = manifold.contacts[0].normal;
                        v3_scalef(&resolve, manifold.contacts[0].penetration);
                        //ric_object_trans(a_obj, &resolve);
                    }

                    bool collide_aabb = ric_aabb_intersects(
                        &a_obj->aabb_world, &b_obj->aabb_world);

                    bool collide_obb = collide_obb_obb(
                        &a_obj->obb, &b_obj->obb, 0);

                    if (collide_sphere)
                    {
                        a_obj->collide_sphere = true;
                        b_obj->collide_sphere = true;
                    }
                    if (collide_aabb)
                    {
                        a_obj->collide_aabb = true;
                        b_obj->collide_aabb = true;
                    }
                    if (collide_obb)
                    {
                        a_obj->collide_obb = true;
                        b_obj->collide_obb = true;
                    }

                    b_id = ric_pack_next_type(b_id, RIC_ASSET_OBJECT);
                }
            }

            a_id = ric_pack_next_type(a_id, RIC_ASSET_OBJECT);
        }
    }
}

bool object_intersects(const struct ric_object *a, const struct ric_object *b,
                       struct manifold *manifold)
{
    // TODO: check sphere, then aabb, then obb
    manifold->body_a = a;
    manifold->body_b = b;
    return DEBUG_sphere_v_sphere(&a->sphere, &b->sphere, manifold);
}

void clash_simulate(struct timmy *timmy)
{
    struct ric_object *obj;
    struct manifold manifold = { 0 };
    const float ground_height = 0.0f;

    for (u32 i = 1; i < ARRAY_COUNT(global_packs); ++i)
    {
        if (!global_packs[i]) continue;

        pkid id = ric_pack_first_type(global_packs[i]->id, RIC_ASSET_OBJECT);
        while (id)
        {
            obj = ric_pack_lookup(id);
            DLB_ASSERT(obj->uid.type == RIC_ASSET_OBJECT);

            // TODO: Give terrain infinite mass and stop arbitrarily
            //       transforming objects into the sky -.-
            if (obj->type == OBJ_TERRAIN)
            {
                id = ric_pack_next_type(id, RIC_ASSET_OBJECT);
                continue;
            }

            obj->resting = false;
            if (obj->resting)
            {
                //if (!(v3_equals(&obj->acc, &VEC3_ZERO) &&
                //      v3_equals(&obj->vel, &VEC3_ZERO) &&
                //      (obj->xform.position.y == 0.0f ||
                //      object_intersects(obj, &timmy->rico, &manifold))))
                if (!(v3_equals(&obj->acc, &VEC3_ZERO) &&
                      v3_equals(&obj->vel, &VEC3_ZERO) &&
                      obj->xform.position.y == ground_height + obj->aabb_world.e.y))
                {
                    obj->resting = false;
                }
            }

            if (!obj->resting)
            {
                obj->acc = GRAVITY;
                v3_add(&obj->vel, &obj->acc);
                v3_add(&obj->xform.position, &obj->vel);
                // TODO: Drag coef

                // TODO: Collision detection
                if (obj->xform.position.y <= ground_height + obj->aabb_world.e.y)
                {
                    struct vec3 p0 = obj->xform.position;
                    v3_sub(&p0, &obj->vel);

                    struct vec3 v0 = obj->vel;
                    struct vec3 v_test;
                    float t = 0.5f;
                    for (int i = 0; i < 10; i++)
                    {
                        obj->xform.position = p0;
                        v_test = v0;
                        v3_scalef(&v_test, t);
                        v3_add(&obj->xform.position, &v_test);
                        ric_object_trans_set(obj, &obj->xform.position);
                        if (obj->xform.position.y <= ground_height + obj->aabb_world.e.y)
                        {
                            t -= t * 0.5f;
                        }
                        else
                        {
                            t += t * 0.5f;
                        }
                    }
                    if (obj->vel.y < 0.0f)
                        obj->vel.y *= -1.0f;
                    obj->vel.y *= COEF_ELASTICITY;

                    v_test = obj->vel;
                    v3_scalef(&v_test, 1.0f - t);
                    v3_add(&obj->xform.position, &v_test);
                    ric_object_trans_set(obj, &obj->xform.position);

                    // TODO: Epsilon (must be bigger than GRAVITY * friction)
                    if (fabs(obj->vel.y) < 0.01f)
                    {
                        obj->acc = VEC3_ZERO;
                        obj->vel = VEC3_ZERO;
                        obj->xform.position.y = ground_height + obj->aabb_world.e.y;
                        //obj->xform.position = VEC3(0.1f, 4.0f, 0.1f);
                        obj->resting = true;
                    }
                }
                else if (object_intersects(obj, &timmy->rico, &manifold))
                {
    #if 0
                    // Continuous collision detection to find time of impact,
                    // doesn't play particularly well with manifold resolution atm.
                    struct vec3 p0 = a_obj->xform.pos;
                    v3_sub(&p0, &a_obj->vel);

                    struct vec3 v0 = a_obj->vel;
                    struct vec3 v_test;
                    float t = 0.5f;
                    for (int i = 0; i < 10; i++)
                    {
                        a_obj->xform.pos = p0;
                        v_test = v0;
                        v3_scalef(&v_test, t);
                        v3_add(&a_obj->xform.pos, &v_test);
                        ric_object_trans_set(a_obj, &a_obj->xform.pos);
                        if (object_intersects(a_obj, &timmy->rico, &manifold))
                        {
                            t -= t * 0.5f;
                        }
                        else
                        {
                            t += t * 0.5f;
                        }
                    }
                    a_obj->vel.y *= -1.0f;
                    a_obj->vel.y *= COEF_ELASTICITY;

                    v_test = a_obj->vel;
                    v3_scalef(&v_test, 1.0f - t);
                    v3_add(&a_obj->xform.pos, &v_test);
                    ric_object_trans_set(a_obj, &a_obj->xform.pos);
    #endif

                    if (manifold.contact_count > 0)
                    {
                        struct vec3 resolve = manifold.contacts[0].normal;
                        v3_scalef(&resolve, manifold.contacts[0].penetration);
                        ric_object_trans(obj, &resolve);
                    }

                    // TODO: Epsilon (must be bigger than GRAVITY * friction)
                    if (fabs(obj->vel.y) < 0.01f)
                    {
                        obj->acc = VEC3_ZERO;
                        obj->vel = VEC3_ZERO;
                        obj->resting = true;
                    }
                }

                DEBUG_render_manifold(&manifold);
                ric_object_trans_set(obj, &obj->xform.position);
            }

            id = ric_pack_next_type(id, RIC_ASSET_OBJECT);
        }
    }
}

void DEBUG_render_color_test()
{
    struct vec4 colors[] = {
        COLOR_RED,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_YELLOW,
        COLOR_CYAN,
        COLOR_MAGENTA,
        COLOR_WHITE,
        COLOR_TRANSPARENT,

        COLOR_DARK_RED,
        COLOR_DARK_GREEN,
        COLOR_DARK_BLUE,
        COLOR_DARK_YELLOW,
        COLOR_DARK_CYAN,
        COLOR_DARK_MAGENTA,
        COLOR_GRAY_5,
        COLOR_TRANSPARENT,

        COLOR_DARK_RED_HIGHLIGHT,
        COLOR_DARK_GREEN_HIGHLIGHT,
        COLOR_DARK_BLUE_HIGHLIGHT,
        COLOR_DARK_YELLOW_HIGHLIGHT,
        COLOR_DARK_CYAN_HIGHLIGHT,
        COLOR_DARK_MAGENTA_HIGHLIGHT,
        COLOR_DARK_WHITE_HIGHLIGHT,
        COLOR_TRANSPARENT,

        COLOR_ORANGE,
        COLOR_PINK,
        COLOR_PURPLE,
        COLOR_LIME,
        COLOR_AQUA,
        COLOR_DODGER,
        COLOR_WHEAT,
        COLOR_BROWN,
        COLOR_BLACK,
    };

    struct ric_aabb color_test = { 0 };
    float x = 0.0f;
    float y = 2.0f;
    const float width = 0.1f;
    const float padding = width * 2.0f;
    for (u32 i = 0; i < ARRAY_COUNT(colors); ++i)
    {
        if (colors[i].a == 0.0f)
        {
            x = 0.0f;
            y += width + padding;
            continue;
        }
        x -= width + padding;
        color_test.c = VEC3(x, y, 0.0f);
        color_test.e = VEC3_1(width);
        ric_prim_draw_aabb(&color_test, &colors[i]);
    }
}

void debug_render_colliders()
{
    for (u32 i = 1; i < ARRAY_COUNT(global_packs); ++i)
    {
        if (!global_packs[i])
            continue;

        pkid id = ric_pack_first_type(global_packs[i]->id, RIC_ASSET_OBJECT);
        while (id)
        {
            struct ric_object *obj = ric_pack_lookup(id);

            struct vec4 col_obb     = COLOR_DARK_RED;
            struct vec4 col_aabb    = COLOR_DARK_ORANGE;
            struct vec4 col_sphere  = COLOR_DARK_YELLOW;

            if (obj->collide_obb)
            {
                col_obb = COLOR_RED;
            }
            else if (obj->collide_aabb)
            {
                col_aabb = COLOR_ORANGE;
            }
            else if (obj->collide_sphere)
            {
                col_sphere = COLOR_YELLOW;
            }

            ric_prim_draw_sphere(&obj->sphere, &col_sphere);
            ric_prim_draw_aabb(&obj->aabb_world, &col_aabb);
            ric_prim_draw_obb(&obj->obb, &col_obb);

            id = ric_pack_next_type(id, RIC_ASSET_OBJECT);
        }
    }
}

void game_toolbar_init()
{
    u32 sprite_count = ARRAY_COUNT(toolbar_sprites);

    toolbar_sheet.tex_id = ric_load_texture_file(RIC_PACK_ID_TRANSIENT, "toolbar",
                                                  "texture/toolbar.tga");
    toolbar_sheet.sprites = toolbar_sprites;
    toolbar_sheet.sprite_count = sprite_count;

    struct ric_texture *tex_sheet = ric_pack_lookup(toolbar_sheet.tex_id);

    u32 sprite_x = 0;
    u32 sprite_y = 0;
    u32 sprite_w = 32;
    u32 sprite_h = 32;
    for (u32 i = 0; i < sprite_count; ++i)
    {
        struct ric_sprite *sprite = &toolbar_sprites[i];
        sprite->sheet = &toolbar_sheet;
        sprite->uvs[0].u = (float)sprite_x / tex_sheet->width;
        sprite->uvs[0].v = (float)(sprite_y + sprite_h) / tex_sheet->height;
        sprite->uvs[1].u = (float)(sprite_x + sprite_w) / tex_sheet->width;
        sprite->uvs[1].v = (float)sprite_y / tex_sheet->height;
        sprite_x += sprite_w;
        if (sprite_x >= tex_sheet->width)
        {
            sprite_x = 0;
            sprite_y += sprite_h;
        }
    }

    for (u32 i = 0; i < TOOLBAR_COUNT; ++i)
    {
        toolbar_tips[i].enabled = true;
        toolbar_tips[i].color = COLOR_GRAY_2; //VEC4(0.2f, 0.4f, 0.8f, 0.9f);
        ric_heiro_build(&toolbar_tips[i].string, 0,
                         &STRL(toolbar_icon_values[i],
                               strlen(toolbar_icon_values[i]) + 1), 0);
    }
}

void toolbar_button_click(const struct ric_ui_event *e)
{
    if ((u32)e->element->metadata == TOOLBAR_CURSOR &&
        e->event_type == RIC_UI_EVENT_LMB_CLICK)
    {
        ric_audio_source_play(&audio_sources[AUDIO_BUTTON]);
    }
}

void game_render_ui_toolbar()
{
    struct ric_ui_hud *toolbar_hud;
    struct ric_ui_button *toolbar_buttons[ARRAY_COUNT(toolbar_sprites)];

    toolbar_hud = ric_ui_hud();
    toolbar_hud->element.padding = PAD(2, 2, 0, 0);
    toolbar_hud->color = COLOR_GRAY_4;

    struct rect margin = PAD(0, 0, 2, 2);

    // Create toolbar buttons
    for (int i = 0; i < ARRAY_COUNT(toolbar_buttons); ++i)
    {
        toolbar_buttons[i] = ric_ui_button(toolbar_hud);
        toolbar_buttons[i]->element.min_size = VEC2I(32, 32);
        toolbar_buttons[i]->element.margin = margin;
        toolbar_buttons[i]->element.metadata = (void *)(TOOLBAR_CURSOR + i);
        toolbar_buttons[i]->element.event = toolbar_button_click;
        toolbar_buttons[i]->color[RIC_UI_STATE_DEFAULT] = COLOR_GRAY_2;
        toolbar_buttons[i]->color[RIC_UI_STATE_HOVERED] = COLOR_ORANGE;
        toolbar_buttons[i]->color[RIC_UI_STATE_PRESSED] = COLOR_DARK_ORANGE;
        toolbar_buttons[i]->sprite = &toolbar_sheet.sprites[i];
        toolbar_buttons[i]->tooltip = &toolbar_tips[i];
    }

    ///////////////////////////////////////////////////////////////////////////
    // Cleanup: Test labels
    static struct ric_heiro_string *label_string = 0;
    if (!label_string)
    {
        ric_heiro_build(&label_string, 0, &STR("Lorem ipsum dolor"), 0);
    }

    const s32 label_pad = 2;
    struct ric_ui_label *label_test1;
    label_test1 = ric_ui_label(toolbar_hud);
    label_test1->element.padding = RECT1(label_pad);
    label_test1->element.margin = margin;
    label_test1->color = COLOR_DARK_RED;
    label_test1->heiro = label_string;

    struct ric_ui_label *label_test2;
    label_test2 = ric_ui_label(toolbar_hud);
    label_test2->element.padding = RECT1(label_pad);
    label_test2->element.margin = margin;
    label_test2->color = COLOR_DARK_BLUE;
    label_test2->heiro = label_string;

    struct ric_ui_label *label_test3;
    label_test3 = ric_ui_label(toolbar_hud);
    label_test3->element.padding = RECT1(label_pad);
    label_test3->element.margin = margin;
    label_test3->color = COLOR_DARK_GREEN;
    label_test3->heiro = label_string;
    ///////////////////////////////////////////////////////////////////////////

    // Layout and render toolbar UI (if it fits)
    if (ric_ui_layout(&toolbar_hud->element, 0, 0, 546, 0))//mouse_x, mouse_y))
    {
        u32 start_x = (SCREEN_WIDTH / 2) - (toolbar_hud->element.size.w / 2);
        ric_ui_draw(&toolbar_hud->element, start_x, 20);
    }
    else
    {
        // Draw silly X to represent layout step failing to fit in min rect
        struct rect x_rect = { 16, 16, 32, 32 };
        float x = SCREEN_X(x_rect.x);
        float y = SCREEN_Y(x_rect.y);
        float w = SCREEN_W(x_rect.w);
        float h = SCREEN_H(x_rect.h);
        ric_prim_draw_line2d(x, y, x + w, y + h, &COLOR_ORANGE);
        ric_prim_draw_line2d(x + w, y, x, y + h, &COLOR_ORANGE);
    }
}

static r32 debug_perc = 0.0f;
void debug_render_ui_stack()
{
    const u32 bar_pad = 2;
    struct ric_ui_hud *hud;
    struct ric_ui_label *label;
    struct ric_ui_progress *progress;
    struct ui_tooltip progress_tip;

    hud = ric_ui_hud();
    hud->element.min_size = VEC2I(100 + bar_pad * 2, 8 + bar_pad * 2);
    hud->element.padding = PAD1(1);
    hud->color = COLOR_DARK_WHITE_HIGHLIGHT;

    struct ric_heiro_string *label_heiro = 0;
    ric_heiro_build(&label_heiro, 0, &STR("UI Stack:"), 0);

    label = ric_ui_label(hud);
    label->heiro = label_heiro;

    progress = ric_ui_progress(hud);
    progress->percent = (ui_stack_ptr - ui_stack) * 100.0f /
                                  sizeof(ui_stack);
    //progress->percent = debug_perc;
    progress->element.min_size = VEC2I(bar_pad * 2 + 100, 8);
    progress->element.margin = RECT1(bar_pad);
    progress->color = COLOR_ORANGE;
    progress->color_bg = COLOR_DARK_ORANGE;

    struct dlb_string progress_tip_str = { 0 };
    char progress_tip_buf[64] = { 0 };
    progress_tip_str.s = progress_tip_buf;
    progress_tip_str.len = 1 +
        snprintf(progress_tip_buf, sizeof(progress_tip_buf) - 1,
                 "%.f%%", progress->percent);
    struct ric_heiro_string *progress_tip_heiro = 0;
    ric_heiro_build(&progress_tip_heiro, 0, &progress_tip_str, 0);

    progress_tip.color = COLOR_DARK_ORANGE;
    progress_tip.enabled = true;
    progress_tip.string = progress_tip_heiro;
    progress->tooltip = &progress_tip;

    ric_ui_line_break(hud);

    // Layout and draw
    if (ric_ui_layout(&hud->element, 300, 60, 0, 0))
    {
        u32 ui_debug_stack_x =
            (SCREEN_WIDTH / 2) - (hud->element.size.w / 2);
        ric_ui_draw(&hud->element, ui_debug_stack_x, 2);
    }

    // TODO: Alloc in frame arena (stack allocator)
    ric_heiro_free(label_heiro);
    ric_heiro_free(progress_tip_heiro);
}

#define LIGHTS_X 12
#define LIGHT_STATES 2
static u32 lights_board[LIGHTS_X][LIGHTS_X];
static struct vec4 colors[LIGHT_STATES][2];

void lights_init()
{
    colors[0][0] = COLOR_GRAY_3;
    colors[0][1] = COLOR_GRAY_2;

    colors[1][0] = COLOR_ORANGE_HIGHLIGHT;
    colors[1][1] = COLOR_ORANGE;
}

void lights_button_click(const struct ric_ui_event *e)
{
    u32 i = (u32)e->element->metadata;
    if (e->event_type == RIC_UI_EVENT_LMB_CLICK)
    {
        //e->element->color = COLOR_ORANGE;
        ric_audio_source_play(&audio_sources[AUDIO_BUTTON]);
        lights_board[i % LIGHTS_X][i / LIGHTS_X]++;
        lights_board[i % LIGHTS_X][i / LIGHTS_X] %= LIGHT_STATES;
    }
}

void game_render_ui_lights()
{
    struct ric_ui_hud *lights_hud;
    struct ric_ui_button *lights_buttons[LIGHTS_X * LIGHTS_X];

    s32 button_w = 32;
    s32 pad = 2;
    s32 margin = 2;

    lights_hud = ric_ui_hud();
    lights_hud->element.padding = PAD(pad, pad, 0, 0);
    lights_hud->color = COLOR_GRAY_4;

    for (int i = 0; i < ARRAY_COUNT(lights_buttons); ++i)
    {
        u32 light_state = lights_board[i % LIGHTS_X][i / LIGHTS_X];

        lights_buttons[i] = ric_ui_button(lights_hud);
        struct ric_ui_button *button = lights_buttons[i];

        button->element.min_size = VEC2I(button_w, button_w);
        button->element.margin = PAD(0, 0, margin, margin);
        button->color[RIC_UI_STATE_HOVERED] = colors[light_state][0];
        button->color[RIC_UI_STATE_DEFAULT] = colors[light_state][1];
        button->element.metadata = (void *)i;
        button->sprite = &toolbar_sheet.sprites[15];
        button->element.event = lights_button_click;
    }

    s32 min_x = (s32)(pad + LIGHTS_X * (button_w + margin));
    s32 min_y = (s32)(pad + LIGHTS_X * (button_w + margin));
    if (ric_ui_layout(&lights_hud->element, min_x, min_y, min_x, min_y))
    {
        s32 start_x = (SCREEN_WIDTH / 2) - (lights_hud->element.size.w / 2);
        ric_ui_draw(&lights_hud->element, start_x, 100);
    }
}

void debug_render_cursor()
{
    //struct rect cursor_rect = { mouse_x - 16, mouse_y - 16, 32, 32 };
    //struct rect cursor_rect = { mouse_x, mouse_y, 32, 32 };
    //ric_prim_draw_rect_tex(&cursor_rect, &COLOR_TRANSPARENT, tex_toolbar);
    //ric_prim_draw_rect(&RECT(mouse_x - 1, mouse_y - 1, 1, 1), &COLOR_RED);
}

void render_editor_ui()
{
    // HACK: Reset ui stack each frame
    // TODO: Reset this in the engine
    rico_ui_reset();

    game_render_ui_toolbar();
    //game_render_ui_lights();
    debug_render_ui_stack();
    debug_render_cursor();
}

#if RICO_DEBUG
#define MEMCHK 250
#endif

#define CHECK_MEM() RICO_ASSERT( \
    text_input.memchk1 == MEMCHK && \
    text_input.memchk2 == MEMCHK);

struct text_input_state
{
    u32 tab_width;
    u32 cursor;
    u32 buffer_len;
    IF_DEBUG(u8 memchk1;)
    char buffer[HEIRO_MAX_LEN];
    IF_DEBUG(u8 memchk2;)
};
struct text_input_state text_input;

void debug_handle_text_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_TEXTINPUT:
            {
                if (text_input.buffer_len < sizeof(text_input.buffer))
                {
                    if (text_input.cursor < text_input.buffer_len)
                    {
                        // Shift all text after cursor right
                        memcpy(text_input.buffer + text_input.cursor + 1,
                               text_input.buffer + text_input.cursor,
                               text_input.buffer_len - text_input.cursor);
                    }
                    text_input.buffer_len++;
                    text_input.buffer[text_input.cursor++] = event.text.text[0];
                    CHECK_MEM();
                }
                break;
            }
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_BACKSPACE:
                    {
                        if (text_input.cursor > 0)
                        {
                            if (text_input.cursor < text_input.buffer_len)
                            {
                                // Shift all text after cursor left
                                memcpy(text_input.buffer + text_input.cursor - 1,
                                       text_input.buffer + text_input.cursor,
                                       text_input.buffer_len - text_input.cursor);
                            }
                            text_input.buffer_len--;
                            text_input.cursor--;
                            CHECK_MEM();
                        }
                        break;
                    }
                    case SDL_SCANCODE_DELETE:
                    {
                        if (text_input.cursor < text_input.buffer_len)
                        {
                            // Shift all text after cursor left
                            memcpy(text_input.buffer + text_input.cursor,
                                   text_input.buffer + text_input.cursor + 1,
                                   text_input.buffer_len - text_input.cursor);
                            text_input.buffer_len--;
                            CHECK_MEM();
                        }
                        break;
                    }
                    case SDL_SCANCODE_RETURN:
                    {
                        if (text_input.buffer_len < sizeof(text_input.buffer))
                        {
                            if (text_input.cursor < text_input.buffer_len)
                            {
                                // Shift all text after cursor right
                                memcpy(text_input.buffer + text_input.cursor + 1,
                                       text_input.buffer + text_input.cursor,
                                       text_input.buffer_len - text_input.cursor);
                            }
                            text_input.buffer_len++;
                            text_input.buffer[text_input.cursor++] = '\n';
                            CHECK_MEM();
                        }
                        break;
                    }
                    case SDL_SCANCODE_TAB:
                    {
                        if (text_input.buffer_len + text_input.tab_width <=
                            sizeof(text_input.buffer))
                        {
                            if (text_input.cursor < text_input.buffer_len)
                            {
                                // Shift all text after cursor right
                                memcpy(text_input.buffer + text_input.cursor + text_input.tab_width,
                                       text_input.buffer + text_input.cursor,
                                       text_input.buffer_len - text_input.cursor);
                            }
                            for (u32 i = 0; i < text_input.tab_width; ++i)
                            {
                                text_input.buffer_len++;
                                text_input.buffer[text_input.cursor++] = ' ';
                            }
                            CHECK_MEM();
                        }
                        break;
                    }
                    case SDL_SCANCODE_LEFT:
                    {
                        if (text_input.cursor > 0)
                        {
                            text_input.cursor--;
                        }
                        break;
                    }
                    case SDL_SCANCODE_RIGHT:
                    {
                        if (text_input.cursor < text_input.buffer_len)
                        {
                            text_input.cursor++;
                        }
                        break;
                    }
                    case SDL_SCANCODE_UP:
                    {
                        // Find BOL (and count columns)
                        u32 column = 0;
                        while (text_input.cursor > 0 &&
                               text_input.buffer[text_input.cursor - 1] != '\n')
                        {
                            text_input.cursor--;
                            column++;
                        }

                        // If cursor at BOF, reset and abort
                        if (text_input.cursor == 0) {
                            text_input.cursor += column;
                            break;
                        }

                        // Eat newline
                        text_input.cursor--;

                        // Save EOL in case prev line is shorter
                        u32 eol = text_input.cursor;

                        // Find BOL of prev line
                        while (text_input.cursor > 0 &&
                               text_input.buffer[text_input.cursor - 1] != '\n')
                        {
                            text_input.cursor--;
                        }

                        // Add column offset (unless EOL comes first)
                        text_input.cursor += MIN(column, eol - text_input.cursor);

                        break;
                    }
                    case SDL_SCANCODE_DOWN:
                    {
                        // TODO: Next line
                        // Scan backward for newline, save count (if EOF, break)
                        // Scan forward for newline
                        // Scan forward until hit count/newline

                        // Already at EOF, nowhere to go
                        if (text_input.cursor == text_input.buffer_len) break;

                        // Find BOL (and count columns)
                        u32 column = 0;
                        while (text_input.cursor > 0 &&
                               text_input.buffer[text_input.cursor - 1] != '\n')
                        {
                            text_input.cursor--;
                            column++;
                        }

                        // Go back to where we started
                        text_input.cursor += column;

                        // Find EOL
                        u32 orig = text_input.cursor;
                        while (text_input.cursor < text_input.buffer_len &&
                               text_input.buffer[text_input.cursor] != '\n')
                        {
                            text_input.cursor++;
                        }

                        // If cursor at EOF, reset and abort
                        if (text_input.cursor == text_input.buffer_len)
                        {
                            text_input.cursor = orig;
                            break;
                        }

                        // Eat newline
                        text_input.cursor++;

                        // Advance until column or EOL
                        while (column > 0 &&
                               text_input.cursor < text_input.buffer_len &&
                               text_input.buffer[text_input.cursor] != '\n')
                        {
                            text_input.cursor++;
                            column--;
                        }

                        break;
                    }
                    case SDL_SCANCODE_HOME:
                    {
                        if (event.key.keysym.mod & KMOD_CTRL)
                        {
                            text_input.cursor = 0;
                        }
                        else
                        {
                            // Go to BOL
                            while (text_input.cursor > 0 &&
                                   text_input.buffer[text_input.cursor - 1] != '\n')
                            {
                                text_input.cursor--;
                            }
                        }
                        break;
                    }
                    case SDL_SCANCODE_END:
                    {
                        if (event.key.keysym.mod & KMOD_CTRL)
                        {
                            text_input.cursor = text_input.buffer_len;
                        }
                        else
                        {
                            // Go to EOL
                            while (text_input.cursor < text_input.buffer_len &&
                                   text_input.buffer[text_input.cursor] != '\n')
                            {
                                text_input.cursor++;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }
}

void debug_render_text_input()
{
    const u32 sx = SCREEN_WIDTH / 2 - (580 / 2);
    const u32 sy = 200;

    struct ric_heiro_string *string = 0;;
    struct rect cursor = { 0 };
    ric_heiro_build(&string, &cursor,
                     &STRL(text_input.buffer, text_input.buffer_len + 1),
                     text_input.cursor);

    const u32 border_width = 2;
    const u32 scroll_w = 10;
    const u32 scroll_h = 30;

    struct rect bounds = { 0 };
    bounds.x = sx;
    bounds.y = sy;
    bounds.w = string->bounds.w + scroll_w;
    bounds.h = string->bounds.h;

    struct rect border = bounds;
    border.x -= border_width;
    border.y -= border_width;
    border.w += border_width * 2;
    border.h += border_width * 2;

    ric_prim_draw_rect(&border, &VEC4(0.2f, 0.2f, 0.2f, 0.5f));
    ric_prim_draw_rect(&bounds, &COLOR_TRANS_BLACK);
    ric_heiro_render(string, sx, sy, &COLOR_GREEN);

    struct rect scroll;
    scroll.x = bounds.x + bounds.w - scroll_w;
    scroll.y = bounds.y;
    scroll.w = scroll_w;
    scroll.h = scroll_h;  // TODO: Calculate based on scroll percentage
    ric_prim_draw_rect(&scroll, &COLOR_ORANGE_HIGHLIGHT);

    cursor.x += sx + string->bounds.x;
    cursor.y += sy + string->bounds.y;
    ric_prim_draw_rect(&cursor, &VEC4(1.0f, 0.0f, 0.0f, 0.5f));

    ric_heiro_free(string);
}

void load_sound(enum audio_type type, const char *filename)
{
    ric_audio_buffer_load_file(&audio_buffers[type], filename);
    ric_audio_source_init(&audio_sources[type]);
    ric_audio_source_buffer(&audio_sources[type], &audio_buffers[type]);
}

void play_sound(enum audio_type type, bool loop)
{
    if (loop)
    {
        ric_audio_source_play_loop(&audio_sources[type]);
    }
    else
    {
        ric_audio_source_play(&audio_sources[type]);
    }
}

void game_setup()
{
    ric_bind_action(CHET_ACTION_TEST_SOUND, RIC_CHORD1(SDL_SCANCODE_Z));
    ric_bind_action(CHET_ACTION_TYPE_NEXT, RIC_CHORD1(SDL_SCANCODE_X));
    ric_bind_action(CHET_ACTION_TYPE_PREV, RIC_CHORD1(SDL_SCANCODE_C));

    load_sound(AUDIO_WELCOME, "audio/welcome.ric");
    load_sound(AUDIO_THUNDER, "audio/thunder_storm.ric");
    load_sound(AUDIO_BUTTON, "audio/bloop2.ric");
    load_sound(AUDIO_VICTORY, "audio/victory.ric");

    ric_audio_volume_set(0.1f);
    play_sound(AUDIO_WELCOME, false);
    play_sound(AUDIO_THUNDER, true);

    game_toolbar_init();
    lights_init();
}

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    UNUSED(panel_1);
    enum ric_error err = RIC_SUCCESS;

	//main_nuklear(argc, argv);
    ric_init();
    pack_build_all();
	pack_load_all();

    game_setup();

    // TODO: Wow this is really dumb. Just make a hash table for name -> pkid
    // HACK: Find Timmy by name and use light/audio flags to determine start-up
    //       state of lighting and audio.
    struct timmy *timmy = ric_pack_lookup_by_name(4, "timmy");
    DLB_ASSERT(timmy->rico.uid.type == RIC_ASSET_OBJECT);
    timmy_state_hacks(timmy->lights_on, timmy->audio_on);

    ric_simulation_pause();

    // TODO: Figure out how to handle UTF-8
    //const char test_str[] = "\u30A2\u30CB\u30E1";
#define STRING_TEST \
    "#=======================================================#\n" \
    "#        ______            _______        _             #\n" \
    "#        |  __ \\ O        |__   __|      | |            #\n" \
    "#        | |__| |_  ___ ___  | | ___  ___| |__          #\n" \
    "#        |  _  /| |/ __/ _ \\ | |/ _ \\/ __| '_ \\         #\n" \
    "#        | | \\ \\| | |_| (_) || |  __/ |__| | | |        #\n" \
    "#        |_|  \\_\\_|\\___\\___/ |_|\\___|\\___|_| |_|        #\n" \
    "#                                                       #\n" \
    "#              Copyright 2018  Dan Bechard              #\n" \
    "#=======================================================#"

    text_input.tab_width = 4;
    text_input.buffer_len = MAX(sizeof(STRING_TEST), 1) - 1;
    text_input.cursor = text_input.buffer_len;
    IF_DEBUG(text_input.memchk1 = MEMCHK);
    memcpy(text_input.buffer, STRING_TEST, text_input.buffer_len);
    IF_DEBUG(text_input.memchk2 = MEMCHK);

    while (!ric_quit())
    {
        ric_mouse_coords(&mouse_x, &mouse_y);

        if (ric_state() == RIC_ENGINE_TEXT_INPUT)
        {
            debug_handle_text_input();
        }
        else
        {
            u32 action;
            while (ric_key_event(&action))
            {
                switch (action)
                {
                    case RIC_ACTION_PLAY_INTERACT:
                        object_interact();
                        break;
                    case CHET_ACTION_TEST_SOUND:
                        ric_audio_source_play(&audio_sources[AUDIO_BUTTON]);
                        debug_perc += 1.0f;
                        break;
                    default:
                        break;
                }

                //if (ric_state_is_edit())
                //{
                //
                //}
                //if (ric_simulation_paused())
                //{
                //    continue;
                //}
            }
        }

        if (!ric_simulation_paused())
        {
            clash_simulate(timmy);
        }

        err = ric_update();
        if (err || ric_quit()) break;

        clash_detect(timmy);

        // Render world
        ric_render_objects();

        //DEBUG_render_color_test();

        // Debug: Move sun away from origin so it doesn't render inside ground
        struct mat4 sun_xform = mat4_init_translate(&VEC3(0.0f, 10.0f, 0.0f));

        // Render light bounds
        for (int i = 0; i < NUM_LIGHT_DIR + NUM_LIGHT_POINT; i++)
        {
            if (!global_prog_pbr->val.frag.lights[i].on)
                continue;

            struct sphere light_sphere = { 0 };
            light_sphere.center = global_prog_pbr->val.frag.lights[i].position;
            light_sphere.r = 0.1f;

            struct vec4 color = VEC4(global_prog_pbr->val.frag.lights[i].color.r,
                                     global_prog_pbr->val.frag.lights[i].color.g,
                                     global_prog_pbr->val.frag.lights[i].color.b,
                                     1.0f);
            if (global_prog_pbr->val.frag.lights[i].type == RIC_LIGHT_DIRECTIONAL)
            {
                ric_prim_draw_sphere_xform(&light_sphere, &color, &sun_xform);
            }
            else
            {
                ric_prim_draw_sphere(&light_sphere, &color);
            }
        }

        // Render shadow lookat axes
        struct vec3 x = VEC3(debug_sun_xform.m[0][0],
                             debug_sun_xform.m[0][1],
                             debug_sun_xform.m[0][2]);
        struct vec3 y = VEC3(debug_sun_xform.m[1][0],
                             debug_sun_xform.m[1][1],
                             debug_sun_xform.m[1][2]);
        struct vec3 z = VEC3(debug_sun_xform.m[2][0],
                             debug_sun_xform.m[2][1],
                             debug_sun_xform.m[2][2]);
        v3_scalef(v3_normalize(&x), 0.2f);
        v3_scalef(v3_normalize(&y), 0.2f);
        v3_scalef(v3_normalize(&z), 0.2f);
        ric_prim_draw_line_xform(&VEC3_ZERO, &x, &COLOR_RED, &sun_xform);
        ric_prim_draw_line_xform(&VEC3_ZERO, &y, &COLOR_GREEN, &sun_xform);
        ric_prim_draw_line_xform(&VEC3_ZERO, &z, &COLOR_BLUE, &sun_xform);
        ric_prim_draw_line_xform(&VEC3_ZERO,
                                  &global_prog_pbr->val.frag.lights[0].directional.direction,
                                  &COLOR_YELLOW, &sun_xform);

        // Cleanup: Debug transform matrices
        //for (u32 y = 0; y < 6; y++)
        //{
        //    for (u32 x = 0; x < 6; x++)
        //    {
        //        struct vec3 p0 = VEC3(-1.5f + 0.1f * x, 1.0f + 0.1f * y, 0.0f);
        //        struct vec3 p1 = VEC3(-1.5f + 0.1f * x, 1.0f + 0.1f * y, -1.0f);
        //        struct vec4 col = VEC4(1.0f / 6 * x, 1.0f / 6 * y, 0.0f, 1.0f);
        //        ric_prim_draw_line(&p0, &p1, &col);
        //
        //        v3_mul_mat4(&p0, &debug_sun_xform);
        //        v3_mul_mat4(&p1, &debug_sun_xform);
        //        ric_prim_draw_line(&p0, &p1, &col);
        //    }
        //}

        if (ric_state_is_edit())
        {
            debug_render_colliders();
        }
        //if (rayviz_sphere.r > 0.0f)
        //{
        //    ric_prim_draw_sphere(&rayviz_sphere, &COLOR_YELLOW);
        //}

        // Render overlays
        if (ric_state_is_edit())
        {
            render_editor_ui();
            ric_render_editor();
        }
        else if (ric_state_is_menu())
        {
            ric_render_editor();
        }

        // Render cursor
        ric_render_crosshair();

        //======================================================================
        if (ric_state() == RIC_ENGINE_TEXT_INPUT)
        {
            debug_render_text_input();
        }
        //======================================================================

        // Swap buffers
        ric_frame_swap();
    }

    ric_audio_source_free(&audio_sources[AUDIO_WELCOME]);
    ric_audio_buffer_free(&audio_buffers[AUDIO_WELCOME]);
    ric_audio_source_free(&audio_sources[AUDIO_BUTTON]);
    ric_audio_buffer_free(&audio_buffers[AUDIO_BUTTON]);
    ric_audio_source_free(&audio_sources[AUDIO_VICTORY]);
    ric_audio_buffer_free(&audio_buffers[AUDIO_VICTORY]);
    ric_audio_source_free(&audio_sources[AUDIO_THUNDER]);
    ric_audio_buffer_free(&audio_buffers[AUDIO_THUNDER]);

    ric_cleanup();
    return err;
}