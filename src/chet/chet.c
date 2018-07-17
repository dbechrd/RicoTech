﻿#include "chet.h"
#include "rico.c"
#include "chet_packs.c"
#include "chet_collision.c"

enum actions
{
    ACTION_RICO_TEST = ACTION_COUNT,
    ACTION_TYPE_NEXT,
    ACTION_TYPE_PREV
};

struct ray_visualizer
{
    struct RICO_object rico;
    u32 lifetime;
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

static struct RICO_audio_buffer audio_buffers[AUDIO_COUNT];
static struct RICO_audio_source audio_sources[AUDIO_COUNT];

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
    TOOLBAR_ICON(GEN_LIST)
    TOOLBAR_COUNT
};
//static const char *toolbar_icon_string[] = { TOOLBAR_ICON(GEN_STRING) };
static const char *toolbar_icon_values[] = { TOOLBAR_ICON(GEN_STRING_VALUES) };

static struct RICO_sprite toolbar_sprites[TOOLBAR_COUNT];
static struct RICO_spritesheet toolbar_sheet;
static struct ui_tooltip toolbar_tips[TOOLBAR_COUNT];

#define PACK_ALPHA 0
#define PACK_CLASH 1

static struct pack_info pack_table[] =
{
    { "packs/alpha.pak" , "packs/alpha.sav" },
    { "packs/clash.pak" , "packs/clash.sav" }
};

//-------------------------------------------------------------------
// TODO: Scale by delta_time properly
#define GRAVITY VEC3(0.0f, -0.0098f, 0.0f);
// TODO: Elastic collision coef
static const float COEF_ELASTICITY = 0.15f;

void pack_build_all()
{
    pack_build_alpha(&pack_table[PACK_ALPHA]);
    pack_build_clash_of_cubes(&pack_table[PACK_CLASH]);
}

int pack_load(u32 pack_idx)
{
    enum RICO_error err;
    err = RICO_pack_load(pack_table[pack_idx].path_pak,
                         &pack_table[pack_idx].pak_id);
    if (err) return err;
    err = RICO_pack_load(pack_table[pack_idx].path_sav,
                         &pack_table[pack_idx].sav_id);
    return err;
}

int pack_load_all()
{
	enum RICO_error err;

    err = pack_load(PACK_ALPHA);
    if (err) return err;
    err = pack_load(PACK_CLASH);
    if (err) return err;

	return err;
}

void timmy_state_hacks(bool lights_on, bool audio_on)
{
    RICO_lighting_enabled = lights_on;
    if (audio_on)
    {
        RICO_audio_unmute();
    }
    else
    {
        RICO_audio_mute();
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
    struct game_panel *panel = RICO_pack_lookup(button->panel_id);
    DLB_ASSERT(button->index < ARRAY_COUNT(panel->buttons));

    pkid mat_on = PKID_GENERATE(3, 4);
    pkid mat_off = PKID_GENERATE(3, 5);

    // Toggle this button
    u32 mat = button->rico.material_id == mat_on ? mat_off : mat_on;
    RICO_object_material_set(&button->rico, mat);

    // Toggle neighbor buttons
    pkid button_id;
    struct game_button *other;
    // UP
    if (button->index >= 3)
    {
        button_id = panel->buttons[button->index - 3];
        other = RICO_pack_lookup(button_id);
        u32 mat = other->rico.material_id == mat_on ? mat_off : mat_on;
        RICO_object_material_set(&other->rico, mat);
    }
    // DOWN
    if (button->index < 6)
    {
        button_id = panel->buttons[button->index + 3];
        other = RICO_pack_lookup(button_id);
        u32 mat = other->rico.material_id == mat_on ? mat_off : mat_on;
        RICO_object_material_set(&other->rico, mat);
    }
    // LEFT
    if (button->index % 3 > 0)
    {
        button_id = panel->buttons[button->index - 1];
        other = RICO_pack_lookup(button_id);
        u32 mat = other->rico.material_id == mat_on ? mat_off : mat_on;
        RICO_object_material_set(&other->rico, mat);
    }
    // RIGHT
    if (button->index % 3 < 2)
    {
        button_id = panel->buttons[button->index + 1];
        other = RICO_pack_lookup(button_id);
        u32 mat = other->rico.material_id == mat_on ? mat_off : mat_on;
        RICO_object_material_set(&other->rico, mat);
    }

    bool victory = true;
    for (u32 i = 0; i < ARRAY_COUNT(panel->buttons); i++)
    {
        struct game_button *button = RICO_pack_lookup(panel->buttons[i]);
        if (button->rico.material_id == mat_on)
        {
            victory = false;
            break;
        }
    }

    RICO_audio_source_play(&audio_sources[AUDIO_BUTTON]);
    if (victory)
    {
        RICO_audio_source_play(&audio_sources[AUDIO_VICTORY]);
    }
}

static struct sphere rayviz_sphere;
void object_interact()
{
    rayviz_sphere.radius = 0.0f;

    pkid obj_id = 0;
    float dist;
    bool collided = RICO_mouse_raycast(&obj_id, &dist);
    if (!collided)
        return;

    float scale = 0.05f;

    struct RICO_camera *camera = RICO_get_camera_hack();
    struct vec3 pos = camera->pos;
    struct vec3 fwd;
    RICO_camera_fwd(&fwd, camera);
    v3_add(&pos, v3_scalef(&fwd, dist));

    // TODO: Subtract (dot(dist, surface.normal) * scale) to place exactly on
    //       surface of mesh? This will place origin of new mesh on surface of
    //       existing mesh.. also need to somehow calculate and subtract
    //       distance from original to edge of new mesh and dot that with the
    //       ray to prevent it from being placed inside.
    rayviz_sphere.orig = pos;
    rayviz_sphere.radius = scale;

    if (dist > 3.0f)
        return;

    struct RICO_object *obj = RICO_pack_lookup(obj_id);
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

bool RICO_object_has_physics(pkid id)
{
    struct RICO_object *obj = RICO_pack_lookup(id);
    // TODO: Implement a proper physics flag
    return !v3_equals(&obj->bbox.min, &obj->bbox.max);
}

pkid RICO_physics_next(pkid id)
{
    pkid next_id = id;
    do
    {
        next_id = RICO_pack_next_loop(next_id);
    } while (next_id && next_id != id && !RICO_object_has_physics(next_id));
    return id;
}

#if 0
void clash_detect()
{
    //struct RICO_bbox *a, b;
    //
    //const int pack_count = ARRAY_COUNT(pack_table);
    //for (int pack_idx_a = 0; pack_idx_a < pack_count; pack_idx_a++)
    //{
    //    struct pack_info *pack_info_a = &pack_table[pack_idx_a];
    //    pack_info_a->pak_id
    //    struct RICO_object *obj_a = RICO_pack_first(
    //}

    pkid a = RICO_pack_first(RICO_pack_active);
    while (RICO_physics_next(a))
    {

    }
}
#endif

bool DEBUG_sphere_v_sphere(const struct sphere *a, const struct sphere *b,
                           struct manifold *manifold)
{
    struct vec3 D = a->orig;
    v3_sub(&D, &b->orig);
    float dist = v3_length(&D);
    float r_sum = a->radius + b->radius;
    bool collide = (dist < r_sum);

    if (collide)
    {
        float pen = r_sum - dist;
        struct vec3 normal = D;
        v3_normalize(&normal);
        struct vec3 position = normal;
        v3_scalef(&position, b->radius - pen);
        v3_add(&position, &b->orig);

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
            manifold_pos.orig = manifold->contacts[i].position;
            manifold_pos.radius = 0.01f;
            RICO_prim_draw_sphere(&manifold_pos, &COLOR_PINK);

            struct vec3 p0 = manifold->contacts[i].position;
            struct vec3 p1 = manifold->contacts[i].normal;
            v3_scalef(&p1, manifold->contacts[i].penetration);
            v3_add(&p1, &p0);
            RICO_prim_draw_line(&p0, &p1, &COLOR_ORANGE);
        }
    }
}

void clash_detect()
{
    for (u32 i = 1; i < ARRAY_COUNT(packs); ++i)
    {
        if (!packs[i])
            continue;

        pkid a_id = RICO_pack_first_type(packs[i]->id, RICO_HND_OBJECT);
        while (a_id)
        {
            struct RICO_object *a_obj = RICO_pack_lookup(a_id);
            if (a_obj->type < RICO_OBJECT_TYPE_COUNT)
            {
                a_id = RICO_pack_next_type(a_id, RICO_HND_OBJECT);
                continue;
            }

            for (u32 j = i; j < ARRAY_COUNT(packs); ++j)
            {
                if (!packs[j])
                    continue;

                pkid b_id;
                if (j == i)
                {
                    b_id = RICO_pack_next_type(a_id, RICO_HND_OBJECT);
                }
                else
                {
                    b_id = RICO_pack_first_type(packs[j]->id, RICO_HND_OBJECT);
                }

                while (b_id)
                {
                    struct RICO_object *b_obj = RICO_pack_lookup(b_id);
                    if (b_obj->type < RICO_OBJECT_TYPE_COUNT)
                    {
                        b_id = RICO_pack_next_type(b_id, RICO_HND_OBJECT);
                        continue;
                    }

                    struct manifold manifold = { 0 };
                    manifold.body_a = a_obj;
                    manifold.body_b = b_obj;
                    bool collide_sphere =
                        DEBUG_sphere_v_sphere(&a_obj->sphere, &b_obj->sphere,
                                              &manifold);
                    // Cleanup: Debug
                    a_obj->collide_sphere = collide_sphere;
                    b_obj->collide_sphere = collide_sphere;

                    DEBUG_render_manifold(&manifold);
                    if (manifold.contact_count > 0)
                    {
                        struct vec3 resolve = manifold.contacts[0].normal;
                        v3_scalef(&resolve, manifold.contacts[0].penetration);
                        //RICO_object_trans(a_obj, &resolve);
                    }

                    bool collide_aabb = a_obj->collide_sphere &&
                        RICO_bbox_intersects(&a_obj->bbox_world,
                                             &b_obj->bbox_world);
                    a_obj->collide_aabb = collide_aabb;
                    b_obj->collide_aabb = collide_aabb;

                    int separating_axis = obb_v_obb(&a_obj->obb, &b_obj->obb);
                    bool collide_obb = a_obj->collide_sphere && (separating_axis == 0);
                    a_obj->collide_obb = collide_obb;
                    b_obj->collide_obb = collide_obb;

                    b_id = RICO_pack_next_type(b_id, RICO_HND_OBJECT);
                }
            }

            a_id = RICO_pack_next_type(a_id, RICO_HND_OBJECT);
        }
    }

    UNUSED("foo");
}

bool object_intersects(const struct RICO_object *a, const struct RICO_object *b,
                       struct manifold *manifold)
{
    // TODO: check sphere, then aabb, then obb
    manifold->body_a = a;
    manifold->body_b = b;
    return DEBUG_sphere_v_sphere(&a->sphere, &b->sphere, manifold);
}

void clash_simulate(struct timmy *timmy)
{
    struct RICO_object *obj;
    struct manifold manifold = { 0 };

    for (u32 i = 1; i < ARRAY_COUNT(packs); ++i)
    {
        if (!packs[i]) continue;

        pkid id = RICO_pack_first_type(packs[i]->id, RICO_HND_OBJECT);
        while (id)
        {
            obj = RICO_pack_lookup(id);

            //if (obj->rico.type != OBJ_SMALL_CUBE)
            //    continue;

            if (obj->resting)
            {
                if (!(v3_equals(&obj->acc, &VEC3_ZERO) &&
                      v3_equals(&obj->vel, &VEC3_ZERO) &&
                      (obj->xform.position.y == 0.0f ||
                      object_intersects(obj, &timmy->rico, &manifold))))
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
                if (obj->xform.position.y <= 0.0f)
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
                        RICO_object_trans_set(obj, &obj->xform.position);
                        if (obj->xform.position.y <= 0.0f)
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
                    RICO_object_trans_set(obj, &obj->xform.position);

                    // TODO: Epsilon (must be bigger than GRAVITY * friction)
                    if (fabs(obj->vel.y) < 0.01f)
                    {
                        obj->acc = VEC3_ZERO;
                        obj->vel = VEC3_ZERO;
                        //obj->rico.xform.position.y = 0.0f;
                        obj->xform.position = VEC3(0.1f, 4.0f, 0.1f);
                        obj->resting = true;
                    }
                }
                else if (object_intersects(obj, &timmy->rico, &manifold))
                {
    #if 0
                    // Continuous collision detection to find time of impact,
                    // doesn't play particularly well with manifold resolution atm.
                    struct vec3 p0 = a_obj->xform.position;
                    v3_sub(&p0, &a_obj->vel);

                    struct vec3 v0 = a_obj->vel;
                    struct vec3 v_test;
                    float t = 0.5f;
                    for (int i = 0; i < 10; i++)
                    {
                        a_obj->xform.position = p0;
                        v_test = v0;
                        v3_scalef(&v_test, t);
                        v3_add(&a_obj->xform.position, &v_test);
                        RICO_object_trans_set(a_obj, &a_obj->xform.position);
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
                    v3_add(&a_obj->xform.position, &v_test);
                    RICO_object_trans_set(a_obj, &a_obj->xform.position);
    #endif

                    if (manifold.contact_count > 0)
                    {
                        struct vec3 resolve = manifold.contacts[0].normal;
                        v3_scalef(&resolve, manifold.contacts[0].penetration);
                        RICO_object_trans(obj, &resolve);
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
                RICO_object_trans_set(obj, &obj->xform.position);
            }

            id = RICO_pack_next_type(id, RICO_HND_OBJECT);
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

    struct RICO_bbox color_test = { 0 };
    float x = 0.0f;
    float y = 2.0f;
    const float width = 0.1f;
    const float padding = width / 2.0f;
    for (u32 i = 0; i < ARRAY_COUNT(colors); ++i)
    {
        if (colors[i].a == 0.0f)
        {
            x = 0.0f;
            y += width + padding;
            continue;
        }
        x -= width + padding;
        color_test.min = VEC3(x - width, y - width,   0.0f);
        color_test.max = VEC3(x        , y        , -width);
        RICO_prim_draw_bbox(&color_test, &colors[i]);
    }
}

void debug_render_bboxes(struct timmy *timmy)
{
    for (u32 i = 1; i < ARRAY_COUNT(packs); ++i)
    {
        if (!packs[i])
            continue;

        pkid id = RICO_pack_first_type(packs[i]->id, RICO_HND_OBJECT);
        while (id)
        {
            struct RICO_object *obj = RICO_pack_lookup(id);

            //struct RICO_bbox obb_bbox = { 0 };
            //obb_bbox.min = obj->rico.bbox.min;
            //v3_mul_mat4(&obb_bbox.min, &obj->rico.xform.matrix);
            //obb_bbox.max = obj->rico.bbox.max;
            //v3_mul_mat4(&obb_bbox.max, &obj->rico.xform.matrix);
            //RICO_prim_draw_bbox(&obb_bbox, &MAT4_IDENT, &COLOR_ORANGE);

            if (obj->collide_obb)
            {
                RICO_prim_draw_sphere(&obj->sphere, &COLOR_TRANS_BLACK);
                RICO_prim_draw_bbox(&obj->bbox_world, &COLOR_TRANS_BLACK);
                RICO_prim_draw_obb(&obj->obb, &COLOR_RED);
            }
            else if (obj->collide_aabb)
            {
                RICO_prim_draw_sphere(&obj->sphere, &COLOR_TRANS_BLACK);
                RICO_prim_draw_bbox(&obj->bbox_world, &COLOR_ORANGE);
                RICO_prim_draw_obb(&obj->obb, &COLOR_TRANS_BLACK);
            }
            else if (obj->collide_sphere)
            {
                RICO_prim_draw_sphere(&obj->sphere, &COLOR_YELLOW);
                RICO_prim_draw_bbox(&obj->bbox_world, &COLOR_TRANS_BLACK);
                RICO_prim_draw_obb(&obj->obb, &COLOR_TRANS_BLACK);
            }
            else
            {
                RICO_prim_draw_sphere(&obj->sphere, &COLOR_TRANS_BLACK);
                RICO_prim_draw_bbox(&obj->bbox_world, &COLOR_TRANS_BLACK);
                RICO_prim_draw_obb(&obj->obb, &COLOR_TRANS_BLACK);
            }

            id = RICO_pack_next_type(id, RICO_HND_OBJECT);
        }
    }
}

void game_toolbar_init()
{
    u32 sprite_count = ARRAY_COUNT(toolbar_sprites);

    toolbar_sheet.tex_id = RICO_load_texture_file(PACK_TRANSIENT, "toolbar",
                                                  "texture/toolbar.tga");
    toolbar_sheet.sprites = toolbar_sprites;
    toolbar_sheet.sprite_count = sprite_count;

    struct RICO_texture *tex_sheet = RICO_pack_lookup(toolbar_sheet.tex_id);

    u32 sprite_x = 0;
    u32 sprite_y = 0;
    u32 sprite_w = 32;
    u32 sprite_h = 32;
    for (u32 i = 0; i < sprite_count; ++i)
    {
        struct RICO_sprite *sprite = &toolbar_sprites[i];
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
        RICO_heiro_build(&toolbar_tips[i].string, 0,
                         &STRL(toolbar_icon_values[i],
                               strlen(toolbar_icon_values[i]) + 1), 0);
    }
}

void toolbar_button_click(const struct RICO_ui_event *e)
{
    if ((u32)e->element->metadata == TOOLBAR_CURSOR &&
        e->event_type == RICO_UI_EVENT_LMB_CLICK)
    {
        RICO_audio_source_play(&audio_sources[AUDIO_BUTTON]);
    }
}

void game_render_ui_toolbar()
{
    struct RICO_ui_hud *toolbar_hud;
    struct RICO_ui_button *toolbar_buttons[ARRAY_COUNT(toolbar_sprites)];

    toolbar_hud = RICO_ui_hud();
    toolbar_hud->element.padding = PAD(2, 2, 0, 0);
    toolbar_hud->color = COLOR_GRAY_4;

    struct rect margin = PAD(0, 0, 2, 2);

    // Create toolbar buttons
    for (int i = 0; i < ARRAY_COUNT(toolbar_buttons); ++i)
    {
        toolbar_buttons[i] = RICO_ui_button(&toolbar_hud->element);
        toolbar_buttons[i]->element.min_size = VEC2I(32, 32);
        toolbar_buttons[i]->element.margin = margin;
        toolbar_buttons[i]->element.metadata = (void *)(TOOLBAR_CURSOR + i);
        toolbar_buttons[i]->element.event = toolbar_button_click;
        toolbar_buttons[i]->color[RICO_UI_BUTTON_DEFAULT] = COLOR_GRAY_2;
        toolbar_buttons[i]->color[RICO_UI_BUTTON_HOVERED] = COLOR_ORANGE;
        toolbar_buttons[i]->color[RICO_UI_BUTTON_PRESSED] = COLOR_DARK_ORANGE;
        toolbar_buttons[i]->sprite = &toolbar_sheet.sprites[i];
        toolbar_buttons[i]->tooltip = &toolbar_tips[i];
    }

    ///////////////////////////////////////////////////////////////////////////
    // Cleanup: Test labels
    static struct RICO_heiro_string *label_string = 0;
    if (!label_string)
    {
        RICO_heiro_build(&label_string, 0, &STR("Lorem ipsum dolor"), 0);
    }

    const s32 label_pad = 2;
    struct RICO_ui_label *label_test1;
    label_test1 = RICO_ui_label(&toolbar_hud->element);
    label_test1->element.padding = RECT1(label_pad);
    label_test1->element.margin = margin;
    label_test1->color = COLOR_DARK_RED;
    label_test1->heiro = label_string;

    struct RICO_ui_label *label_test2;
    label_test2 = RICO_ui_label(&toolbar_hud->element);
    label_test2->element.padding = RECT1(label_pad);
    label_test2->element.margin = margin;
    label_test2->color = COLOR_DARK_BLUE;
    label_test2->heiro = label_string;

    struct RICO_ui_label *label_test3;
    label_test3 = RICO_ui_label(&toolbar_hud->element);
    label_test3->element.padding = RECT1(label_pad);
    label_test3->element.margin = margin;
    label_test3->color = COLOR_DARK_GREEN;
    label_test3->heiro = label_string;
    ///////////////////////////////////////////////////////////////////////////

    // Layout and render toolbar UI (if it fits)
    if (RICO_ui_layout(&toolbar_hud->element, 0, 0, 546, 0))//mouse_x, mouse_y))
    {
        u32 start_x = (SCREEN_WIDTH / 2) - (toolbar_hud->element.size.w / 2);
        RICO_ui_draw(&toolbar_hud->element, start_x, 20);
    }
    else
    {
        // Draw silly X to represent layout step failing to fit in min rect
        struct rect x_rect = { 16, 16, 32, 32 };
        float x = SCREEN_X(x_rect.x);
        float y = SCREEN_Y(x_rect.y);
        float w = SCREEN_W(x_rect.w);
        float h = SCREEN_H(x_rect.h);
        RICO_prim_draw_line2d(x, y, x + w, y + h, &COLOR_ORANGE);
        RICO_prim_draw_line2d(x + w, y, x, y + h, &COLOR_ORANGE);
    }
}

void debug_render_ui_stack()
{
    struct RICO_ui_hud *ui_debug_usage_hud;
    struct RICO_ui_label *ui_debug_usage_bar;

    ui_debug_usage_hud = RICO_ui_hud();

    // TODO: Make progress bar control
    ui_debug_usage_bar = RICO_ui_label(&ui_debug_usage_hud->element);

    u32 ui_stack_used = (ui_stack_ptr - ui_stack) * 100 / sizeof(ui_stack);
    u32 bar_pad = 2;
    ui_debug_usage_bar->element.min_size = VEC2I(ui_stack_used, 8);
    ui_debug_usage_bar->element.margin = RECT1(bar_pad);
    ui_debug_usage_bar->color = COLOR_DARK_YELLOW;
    ui_debug_usage_hud->element.min_size = VEC2I(100 + bar_pad * 2,
                                                 8 + bar_pad * 2);
    ui_debug_usage_hud->color = COLOR_DARK_WHITE_HIGHLIGHT;

    // Layout and draw
    if (RICO_ui_layout(&ui_debug_usage_hud->element, 300, 60, 0, 0))
    {
        u32 ui_debug_stack_x =
            (SCREEN_WIDTH / 2) - (ui_debug_usage_hud->element.size.w / 2);
        RICO_ui_draw(&ui_debug_usage_hud->element, ui_debug_stack_x, 2);
    }
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

void lights_button_click(const struct RICO_ui_event *e)
{
    u32 i = (u32)e->element->metadata;
    if (e->event_type == RICO_UI_EVENT_LMB_CLICK)
    {
        //e->element->color = COLOR_ORANGE;
        RICO_audio_source_play(&audio_sources[AUDIO_BUTTON]);
        lights_board[i % LIGHTS_X][i / LIGHTS_X]++;
        lights_board[i % LIGHTS_X][i / LIGHTS_X] %= LIGHT_STATES;
    }
}

void game_render_ui_lights()
{
    struct RICO_ui_hud *lights_hud;
    struct RICO_ui_button *lights_buttons[LIGHTS_X * LIGHTS_X];

    s32 button_w = 32;
    s32 pad = 2;
    s32 margin = 2;

    lights_hud = RICO_ui_hud();
    lights_hud->element.padding = PAD(pad, pad, 0, 0);
    lights_hud->color = COLOR_GRAY_4;

    for (int i = 0; i < ARRAY_COUNT(lights_buttons); ++i)
    {
        u32 light_state = lights_board[i % LIGHTS_X][i / LIGHTS_X];

        lights_buttons[i] = RICO_ui_button(&lights_hud->element);
        struct RICO_ui_button *button = lights_buttons[i];

        button->element.min_size = VEC2I(button_w, button_w);
        button->element.margin = PAD(0, 0, margin, margin);
        button->color[RICO_UI_BUTTON_HOVERED] = colors[light_state][0];
        button->color[RICO_UI_BUTTON_DEFAULT] = colors[light_state][1];
        button->element.metadata = (void *)i;
        button->sprite = &toolbar_sheet.sprites[15];
        button->element.event = lights_button_click;
    }

    s32 min_x = (s32)(pad + LIGHTS_X * (button_w + margin));
    s32 min_y = (s32)(pad + LIGHTS_X * (button_w + margin));
    if (RICO_ui_layout(&lights_hud->element, min_x, min_y, min_x, min_y))
    {
        s32 start_x = (SCREEN_WIDTH / 2) - (lights_hud->element.size.w / 2);
        RICO_ui_draw(&lights_hud->element, start_x, 100);
    }
}

void debug_render_cursor()
{
    //struct rect cursor_rect = { mouse_x - 16, mouse_y - 16, 32, 32 };
    //struct rect cursor_rect = { mouse_x, mouse_y, 32, 32 };
    //RICO_prim_draw_rect_tex(&cursor_rect, &COLOR_TRANSPARENT, tex_toolbar);
    //RICO_prim_draw_rect(&RECT(mouse_x - 1, mouse_y - 1, 1, 1), &COLOR_RED);
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

    struct RICO_heiro_string *string = 0;;
    struct rect cursor = { 0 };
    RICO_heiro_build(&string, &cursor,
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

    RICO_prim_draw_rect(&border, &VEC4(0.2f, 0.2f, 0.2f, 0.5f));
    RICO_prim_draw_rect(&bounds, &COLOR_TRANS_BLACK);
    RICO_heiro_render(string, sx, sy, &COLOR_GREEN);

    struct rect scroll;
    scroll.x = bounds.x + bounds.w - scroll_w;
    scroll.y = bounds.y;
    scroll.w = scroll_w;
    scroll.h = scroll_h;  // TODO: Calculate based on scroll percentage
    RICO_prim_draw_rect(&scroll, &COLOR_ORANGE_HIGHLIGHT);

    cursor.x += sx + string->bounds.x;
    cursor.y += sy + string->bounds.y;
    RICO_prim_draw_rect(&cursor, &VEC4(1.0f, 0.0f, 0.0f, 0.5f));

    RICO_heiro_free(string);
}

void load_sound(enum audio_type type, const char *filename)
{
    RICO_audio_buffer_load_file(&audio_buffers[type], filename);
    RICO_audio_source_init(&audio_sources[type]);
    RICO_audio_source_buffer(&audio_sources[type], &audio_buffers[type]);
}

void play_sound(enum audio_type type, bool loop)
{
    if (loop)
    {
        RICO_audio_source_play_loop(&audio_sources[type]);
    }
    else
    {
        RICO_audio_source_play(&audio_sources[type]);
    }
}

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    UNUSED(panel_1);
    enum RICO_error err = SUCCESS;

	//main_nuklear(argc, argv);
    RICO_init();
    pack_build_all();
	pack_load_all();

    ric_test();

    RICO_bind_action(ACTION_RICO_TEST, CHORD_REPEAT1(SDL_SCANCODE_Z));
    RICO_bind_action(ACTION_TYPE_NEXT, CHORD1(SDL_SCANCODE_X));
    RICO_bind_action(ACTION_TYPE_PREV, CHORD1(SDL_SCANCODE_C));

    load_sound(AUDIO_WELCOME, "audio/welcome.ric");
    load_sound(AUDIO_THUNDER, "audio/thunder_storm.ric");
    load_sound(AUDIO_BUTTON, "audio/bloop2.ric");
    load_sound(AUDIO_VICTORY, "audio/victory.ric");

    RICO_audio_volume_set(0.1f);
    play_sound(AUDIO_WELCOME, false);
    play_sound(AUDIO_THUNDER, true);

    game_toolbar_init();
    lights_init();

    // HACK: Find Timmy by name and use light/audio flags to determine start-up
    //       state of lighting and audio.
    struct timmy *timmy =
        RICO_pack_lookup_by_name(pack_table[PACK_ALPHA].sav_id, "timmy");
    DLB_ASSERT(timmy);
    timmy_state_hacks(timmy->lights_on, timmy->audio_on);

    RICO_simulation_pause();

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

    while (!RICO_quit())
    {
        RICO_mouse_coords(&mouse_x, &mouse_y);

        if (RICO_state() == STATE_TEXT_INPUT)
        {
            debug_handle_text_input();
        }
        else
        {
            u32 action;
            while (RICO_key_event(&action))
            {
                if (RICO_state_is_edit() || RICO_simulation_paused())
                    continue;

                switch (action)
                {
                    case ACTION_PLAY_INTERACT:
                        object_interact();
                        break;
                    case ACTION_RICO_TEST:
                        RICO_audio_source_play(&audio_sources[AUDIO_BUTTON]);
                        break;
                    default:
                        break;
                }
            }
        }

        if (!RICO_simulation_paused())
        {
            clash_simulate(timmy);
        }

        err = RICO_update();
        if (err || RICO_quit()) break;

        clash_detect(timmy);

        // Render world
        RICO_render_objects();
        debug_render_bboxes(timmy);
        if (rayviz_sphere.radius > 0.0f)
        {
            RICO_prim_draw_sphere(&rayviz_sphere, &COLOR_YELLOW);
        }

        // Render overlays
        if (RICO_state_is_edit())
        {
            render_editor_ui();
            RICO_render_editor();
        }
        else if (RICO_state_is_menu())
        {
            RICO_render_editor();
        }

        // Render light bounds
        for (int i = 0; i < NUM_LIGHT_DIR + NUM_LIGHT_POINT; i++)
        {
            if (!prog_pbr->frag.lights[i].enabled)
                continue;

            struct sphere light_sphere = { 0 };
            light_sphere.orig = prog_pbr->frag.lights[i].pos;
            light_sphere.radius = 0.1f;

            struct vec4 color = VEC4(prog_pbr->frag.lights[i].color.r,
                                     prog_pbr->frag.lights[i].color.g,
                                     prog_pbr->frag.lights[i].color.b,
                                     1.0f);
            RICO_prim_draw_sphere(&light_sphere, &color);
        }

        // Render cursor
        RICO_render_crosshair();

        //======================================================================
        if (RICO_state() == STATE_TEXT_INPUT)
        {
            debug_render_text_input();
        }
        //======================================================================

        // Swap buffers
        RICO_frame_swap();
    }

    RICO_audio_source_free(&audio_sources[AUDIO_WELCOME]);
    RICO_audio_buffer_free(&audio_buffers[AUDIO_WELCOME]);
    RICO_audio_source_free(&audio_sources[AUDIO_BUTTON]);
    RICO_audio_buffer_free(&audio_buffers[AUDIO_BUTTON]);
    RICO_audio_source_free(&audio_sources[AUDIO_VICTORY]);
    RICO_audio_buffer_free(&audio_buffers[AUDIO_VICTORY]);
    RICO_audio_source_free(&audio_sources[AUDIO_THUNDER]);
    RICO_audio_buffer_free(&audio_buffers[AUDIO_THUNDER]);

    RICO_cleanup();
    return err;
}