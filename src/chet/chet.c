#include "chet.h"
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

enum toolbar_icon {
    TOOLBAR_CURSOR = 1,
    TOOLBAR_TRANSLATE,
    TOOLBAR_ROTATE,
    TOOLBAR_SCALE,
    TOOLBAR_MESH,
    TOOLBAR_TEXTURE,
    TOOLBAR_NEW,
    TOOLBAR_COPY,
    TOOLBAR_DELETE,
    TOOLBAR_UNDO,
    TOOLBAR_REDO,
    TOOLBAR_SAVE,
    TOOLBAR_EXIT
};

static struct RICO_sprite toolbar_sprites[16];
static struct RICO_spritesheet toolbar_sheet;

#define PACK_ALPHA 0
#define PACK_CLASH 1

static struct pack_info pack_table[] =
{
    { "packs/alpha.pak" , "packs/alpha.sav", 0, 0 },
    { "packs/clash.pak" , "packs/clash.sav", 0, 0 }
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

    RICO_pack_active = pack_table[PACK_CLASH].sav_id;
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

void clash_detect(struct timmy *timmy)
{
    pkid id = RICO_pack_first_type(pack_table[PACK_CLASH].sav_id,
                                   RICO_HND_OBJECT);

    while (id)
    {
        struct small_cube *obj = RICO_pack_lookup(id);
        if (obj->rico.type != OBJ_SMALL_CUBE)
            continue;

        struct manifold manifold = { 0 };
        manifold.body_a = &obj->rico;
        manifold.body_b = &timmy->rico;
        obj->collide_sphere = DEBUG_sphere_v_sphere(&obj->rico.sphere,
                                                    &timmy->rico.sphere,
                                                    &manifold);
        DEBUG_render_manifold(&manifold);
        if (manifold.contact_count > 0)
        {
            struct vec3 resolve = manifold.contacts[0].normal;
            v3_scalef(&resolve, manifold.contacts[0].penetration);
            RICO_object_trans(&obj->rico, &resolve);
        }

        //obj->collide_aabb = obj->collide_sphere &&
        //    RICO_bbox_intersects(&obj->rico.bbox_world,
        //                         &timmy->rico.bbox_world);
        //
        //int separating_axis = obb_v_obb(&obj->rico.obb, &timmy->rico.obb);
        //obj->collide_obb = obj->collide_sphere && (separating_axis == 0);

        id = RICO_pack_next_type(id, RICO_HND_OBJECT);
    }
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
    pkid id = RICO_pack_first_type(pack_table[PACK_CLASH].sav_id,
                                   RICO_HND_OBJECT);

    struct small_cube *obj;
    struct manifold manifold = { 0 };

    while (id)
    {
        obj = RICO_pack_lookup(id);

        //if (obj->rico.type != OBJ_SMALL_CUBE)
        //    continue;

        if (obj->resting)
        {
            if (!(v3_equals(&obj->acc, &VEC3_ZERO) &&
                  v3_equals(&obj->vel, &VEC3_ZERO) &&
                  (obj->rico.xform.position.y == 0.0f ||
                  object_intersects(&obj->rico, &timmy->rico, &manifold))))
            {
                obj->resting = false;
            }
        }

        if (!obj->resting)
        {
            obj->acc = GRAVITY;
            v3_add(&obj->vel, &obj->acc);
            v3_add(&obj->rico.xform.position, &obj->vel);
            // TODO: Drag coef

            // TODO: Collision detection
            if (obj->rico.xform.position.y <= 0.0f)
            {
                struct vec3 p0 = obj->rico.xform.position;
                v3_sub(&p0, &obj->vel);

                struct vec3 v0 = obj->vel;
                struct vec3 v_test;
                float t = 0.5f;
                for (int i = 0; i < 10; i++)
                {
                    obj->rico.xform.position = p0;
                    v_test = v0;
                    v3_scalef(&v_test, t);
                    v3_add(&obj->rico.xform.position, &v_test);
                    RICO_object_trans_set(&obj->rico,
                                          &obj->rico.xform.position);
                    if (obj->rico.xform.position.y <= 0.0f)
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
                v3_add(&obj->rico.xform.position, &v_test);
                RICO_object_trans_set(&obj->rico, &obj->rico.xform.position);

                // TODO: Epsilon (must be bigger than GRAVITY * friction)
                if (fabs(obj->vel.y) < 0.01f)
                {
                    obj->acc = VEC3_ZERO;
                    obj->vel = VEC3_ZERO;
                    //obj->rico.xform.position.y = 0.0f;
                    obj->rico.xform.position = VEC3(0.1f, 4.0f, 0.1f);
                    obj->resting = true;
                }
            }
            else if (object_intersects(&obj->rico, &timmy->rico, &manifold))
            {
#if 0
                // Continuous collision detection to find time of impact,
                // doesn't play particularly well with manifold resolution atm.
                struct vec3 p0 = obj->rico.xform.position;
                v3_sub(&p0, &obj->vel);

                struct vec3 v0 = obj->vel;
                struct vec3 v_test;
                float t = 0.5f;
                for (int i = 0; i < 10; i++)
                {
                    obj->rico.xform.position = p0;
                    v_test = v0;
                    v3_scalef(&v_test, t);
                    v3_add(&obj->rico.xform.position, &v_test);
                    RICO_object_trans_set(&obj->rico,
                                          &obj->rico.xform.position);
                    if (object_intersects(&obj->rico, &timmy->rico, &manifold))
                    {
                        t -= t * 0.5f;
                    }
                    else
                    {
                        t += t * 0.5f;
                    }
                }
                obj->vel.y *= -1.0f;
                obj->vel.y *= COEF_ELASTICITY;

                v_test = obj->vel;
                v3_scalef(&v_test, 1.0f - t);
                v3_add(&obj->rico.xform.position, &v_test);
                RICO_object_trans_set(&obj->rico, &obj->rico.xform.position);
#endif

                if (manifold.contact_count > 0)
                {
                    struct vec3 resolve = manifold.contacts[0].normal;
                    v3_scalef(&resolve, manifold.contacts[0].penetration);
                    RICO_object_trans(&obj->rico, &resolve);
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
            RICO_object_trans_set(&obj->rico, &obj->rico.xform.position);
        }

        id = RICO_pack_next_type(id, RICO_HND_OBJECT);
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

void DEBUG_render_bboxes(struct timmy *timmy)
{
    RICO_prim_draw_sphere(&timmy->rico.sphere, &COLOR_DARK_WHITE_HIGHLIGHT);
    RICO_prim_draw_bbox(&timmy->rico.bbox_world, &COLOR_AQUA);
    RICO_prim_draw_obb(&timmy->rico.obb, &COLOR_LIME);

    pkid id = RICO_pack_first_type(pack_table[PACK_CLASH].sav_id,
                                   RICO_HND_OBJECT);
    while (id)
    {
        struct small_cube *obj = RICO_pack_lookup(id);

        switch (obj->rico.type)
        {
            case OBJ_SMALL_CUBE:  // fall through
            case OBJ_TIMMY:
            {
                //struct RICO_bbox obb_bbox = { 0 };
                //obb_bbox.min = obj->rico.bbox.min;
                //v3_mul_mat4(&obb_bbox.min, &obj->rico.xform.matrix);
                //obb_bbox.max = obj->rico.bbox.max;
                //v3_mul_mat4(&obb_bbox.max, &obj->rico.xform.matrix);
                //RICO_prim_draw_bbox(&obb_bbox, &MAT4_IDENT, &COLOR_ORANGE);

                if (obj->collide_obb)
                {
                    RICO_prim_draw_sphere(&obj->rico.sphere,
                                          &COLOR_DARK_WHITE_HIGHLIGHT);
                    RICO_prim_draw_bbox(&obj->rico.bbox_world,
                                        &COLOR_DARK_WHITE_HIGHLIGHT);
                    RICO_prim_draw_obb(&obj->rico.obb, &COLOR_RED);
                }
                else if (obj->collide_aabb)
                {
                    RICO_prim_draw_sphere(&obj->rico.sphere,
                                          &COLOR_DARK_WHITE_HIGHLIGHT);
                    RICO_prim_draw_bbox(&obj->rico.bbox_world, &COLOR_ORANGE);
                    RICO_prim_draw_obb(&obj->rico.obb,
                                       &COLOR_DARK_WHITE_HIGHLIGHT);
                }
                else if (obj->collide_sphere)
                {
                    RICO_prim_draw_sphere(&obj->rico.sphere, &COLOR_YELLOW);
                    RICO_prim_draw_bbox(&obj->rico.bbox_world,
                                        &COLOR_DARK_WHITE_HIGHLIGHT);
                    RICO_prim_draw_obb(&obj->rico.obb,
                                       &COLOR_DARK_WHITE_HIGHLIGHT);
                }
                else
                {
                    RICO_prim_draw_sphere(&obj->rico.sphere,
                                          &COLOR_DARK_WHITE_HIGHLIGHT);
                    RICO_prim_draw_bbox(&obj->rico.bbox_world, &COLOR_AQUA);
                    RICO_prim_draw_obb(&obj->rico.obb, &COLOR_LIME);
                }
                break;
            }
            default:
                break;
        }

        id = RICO_pack_next_type(id, RICO_HND_OBJECT);
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
}

static u32 pointless_buttons = 0;

void toolbar_button_click(const struct RICO_ui_event *e)
{
    if ((u32)e->element->metadata == TOOLBAR_CURSOR &&
        e->event_type == RICO_UI_EVENT_LMB_CLICK)
    {
        //e->element->color = COLOR_ORANGE;
        RICO_audio_source_play(&audio_sources[AUDIO_BUTTON]);
        pointless_buttons++;
    }
}

void game_render_ui_toolbar()
{
    struct RICO_ui_hud *toolbar_hud;
    struct RICO_ui_button *toolbar_buttons[ARRAY_COUNT(toolbar_sprites)];

    toolbar_hud = RICO_ui_hud();
    toolbar_hud->element.padding = PAD(2, 2, 0, 0);
    toolbar_hud->color = COLOR_GRAY_4;

    for (int i = 0; i < ARRAY_COUNT(toolbar_buttons); ++i)
    {
        toolbar_buttons[i] = RICO_ui_button(&toolbar_hud->element);
        toolbar_buttons[i]->element.min_size = VEC2I(32, 32);
        toolbar_buttons[i]->element.margin = PAD(0, 0, 2, 2);
        toolbar_buttons[i]->element.metadata = (void *)(TOOLBAR_CURSOR + i);
        toolbar_buttons[i]->element.event = toolbar_button_click;
        toolbar_buttons[i]->color[RICO_UI_BUTTON_DEFAULT] = COLOR_GRAY_2;
        toolbar_buttons[i]->color[RICO_UI_BUTTON_HOVERED] = COLOR_ORANGE;
        toolbar_buttons[i]->color[RICO_UI_BUTTON_PRESSED] = COLOR_DARK_ORANGE;
        toolbar_buttons[i]->sprite = &toolbar_sheet.sprites[i];
    }

    for (u32 i = 0; i < pointless_buttons; ++i)
    {
        struct RICO_ui_button *button = RICO_ui_button(&toolbar_hud->element);
        button->element.min_size = VEC2I(32, 32);
        button->element.margin = PAD(0, 0, 2, 2);
        button->color[RICO_UI_BUTTON_DEFAULT] = COLOR_GRAY_2;
        button->color[RICO_UI_BUTTON_HOVERED] = COLOR_ORANGE;
        button->color[RICO_UI_BUTTON_PRESSED] = COLOR_DARK_ORANGE;
        button->sprite = &toolbar_sheet.sprites[0];
    }

    if (RICO_ui_layout(&toolbar_hud->element, 0, 0, 546, 0))
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
    //pack_build_all();
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

    u32 tab_width = 4;
    u32 input_len = MAX(sizeof(STRING_TEST), 1) - 1;
    u32 input_cursor = input_len;
#if RICO_DEBUG
    #define MEMCHK 250
#endif
    IF_DEBUG(u8 memchk1 = MEMCHK);
    char input_buffer[HEIRO_MAX_LEN] = STRING_TEST;
    IF_DEBUG(u8 memchk2 = MEMCHK);

    while (!RICO_quit())
    {
        RICO_mouse_coords(&mouse_x, &mouse_y);

        if (RICO_state() == STATE_TEXT_INPUT)
        {
            // TODO: Refactor this out into state_text_input run handler
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type) {
                case SDL_TEXTINPUT:
                {
                    if (input_len < sizeof(input_buffer))
                    {
                        if (input_cursor < input_len)
                        {
                            // Shift all text after cursor right
                            memcpy(input_buffer + input_cursor + 1,
                                   input_buffer + input_cursor,
                                   input_len - input_cursor);
                        }
                        input_len++;
                        input_buffer[input_cursor++] = event.text.text[0];
                        RICO_ASSERT(memchk1 == MEMCHK && memchk2 == MEMCHK);
                    }
                    break;
                }
                case SDL_KEYDOWN:
                {
                    switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_BACKSPACE:
                    {
                        if (input_cursor > 0)
                        {
                            if (input_cursor < input_len)
                            {
                                // Shift all text after cursor left
                                memcpy(input_buffer + input_cursor - 1,
                                       input_buffer + input_cursor,
                                       input_len - input_cursor);
                            }
                            input_len--;
                            input_cursor--;
                            RICO_ASSERT(memchk1 == MEMCHK && memchk2 == MEMCHK);
                        }
                        break;
                    }
                    case SDL_SCANCODE_DELETE:
                    {
                        if (input_cursor < input_len)
                        {
                            // Shift all text after cursor left
                            memcpy(input_buffer + input_cursor,
                                    input_buffer + input_cursor + 1,
                                    input_len - input_cursor);
                            input_len--;
                            RICO_ASSERT(memchk1 == MEMCHK && memchk2 == MEMCHK);
                        }
                        break;
                    }
                    case SDL_SCANCODE_RETURN:
                    {
                        if (input_len < sizeof(input_buffer))
                        {
                            if (input_cursor < input_len)
                            {
                                // Shift all text after cursor right
                                memcpy(input_buffer + input_cursor + 1,
                                       input_buffer + input_cursor,
                                       input_len - input_cursor);
                            }
                            input_len++;
                            input_buffer[input_cursor++] = '\n';
                            RICO_ASSERT(memchk1 == MEMCHK && memchk2 == MEMCHK);
                        }
                        break;
                    }
                    case SDL_SCANCODE_TAB:
                    {
                        if (input_len + tab_width <= sizeof(input_buffer))
                        {
                            if (input_cursor < input_len)
                            {
                                // Shift all text after cursor right
                                memcpy(input_buffer + input_cursor + tab_width,
                                       input_buffer + input_cursor,
                                       input_len - input_cursor);
                            }
                            for (u32 i = 0; i < tab_width; ++i)
                            {
                                input_len++;
                                input_buffer[input_cursor++] = ' ';
                            }
                            RICO_ASSERT(memchk1 == MEMCHK && memchk2 == MEMCHK);
                        }
                        break;
                    }
                    case SDL_SCANCODE_LEFT:
                    {
                        if (input_cursor > 0)
                        {
                            input_cursor--;
                        }
                        break;
                    }
                    case SDL_SCANCODE_RIGHT:
                    {
                        if (input_cursor < input_len)
                        {
                            input_cursor++;
                        }
                        break;
                    }
                    case SDL_SCANCODE_UP:
                    {
                        // Find BOL (and count columns)
                        u32 column = 0;
                        while (input_cursor > 0 &&
                               input_buffer[input_cursor - 1] != '\n')
                        {
                            input_cursor--;
                            column++;
                        }

                        // If cursor at BOF, reset and abort
                        if (input_cursor == 0) {
                            input_cursor += column;
                            break;
                        }

                        // Eat newline
                        input_cursor--;

                        // Save EOL in case prev line is shorter
                        u32 eol = input_cursor;

                        // Find BOL of prev line
                        while (input_cursor > 0 &&
                               input_buffer[input_cursor - 1] != '\n')
                        {
                            input_cursor--;
                        }

                        // Add column offset (unless EOL comes first)
                        input_cursor += MIN(column, eol - input_cursor);

                        break;
                    }
                    case SDL_SCANCODE_DOWN:
                    {
                        // TODO: Next line
                        // Scan backward for newline, save count (if EOF, break)
                        // Scan forward for newline
                        // Scan forward until hit count/newline

                        // Already at EOF, nowhere to go
                        if (input_cursor == input_len) break;

                        // Find BOL (and count columns)
                        u32 column = 0;
                        while (input_cursor > 0 &&
                               input_buffer[input_cursor - 1] != '\n')
                        {
                            input_cursor--;
                            column++;
                        }

                        // Go back to where we started
                        input_cursor += column;

                        // Find EOL
                        u32 orig = input_cursor;
                        while (input_cursor < input_len &&
                               input_buffer[input_cursor] != '\n')
                        {
                            input_cursor++;
                        }

                        // If cursor at EOF, reset and abort
                        if (input_cursor == input_len)
                        {
                            input_cursor = orig;
                            break;
                        }

                        // Eat newline
                        input_cursor++;

                        // Advance until column or EOL
                        while (column > 0 &&
                               input_cursor < input_len &&
                               input_buffer[input_cursor] != '\n')
                        {
                            input_cursor++;
                            column--;
                        }

                        break;
                    }
                    case SDL_SCANCODE_HOME:
                    {
                        if (event.key.keysym.mod & KMOD_CTRL)
                        {
                            input_cursor = 0;
                        }
                        else
                        {
                            // Go to BOL
                            while (input_cursor > 0 &&
                                   input_buffer[input_cursor - 1] != '\n')
                            {
                                input_cursor--;
                            }
                        }
                        break;
                    }
                    case SDL_SCANCODE_END:
                    {
                        if (event.key.keysym.mod & KMOD_CTRL)
                        {
                            input_cursor = input_len;
                        }
                        else
                        {
                            // Go to EOL
                            while (input_cursor < input_len &&
                                   input_buffer[input_cursor] != '\n')
                            {
                                input_cursor++;
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
        if (err) break;

        //clash_detect(timmy);

        // Render world
        RICO_render_objects();
        DEBUG_render_bboxes(timmy);
        if (rayviz_sphere.radius > 0.0f)
            RICO_prim_draw_sphere(&rayviz_sphere, &COLOR_YELLOW);

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

        // Render cursor
        RICO_render_crosshair();

        //======================================================================
        if (RICO_state() == STATE_TEXT_INPUT)
        {
            struct rect bounds = { 0 };
            struct rect cursor = { 0 };
            RICO_heiro_build(&bounds, &cursor, input_buffer, input_len,
                             input_cursor);

            const u32 sx = SCREEN_WIDTH / 2 - (580 / 2);
            const u32 sy = 200;
            const u32 border_width = 2;
            const u32 scroll_w = 10;

            bounds.x += sx;
            bounds.y += sy;
            bounds.w += scroll_w;
            cursor.x += sx;
            cursor.y += sy;

            struct rect border = bounds;
            border.x -= border_width;
            border.y -= border_width;
            border.w += border_width * 2;
            border.h += border_width * 2;

            struct rect scroll;
            scroll.x = bounds.x + bounds.w - scroll_w;
            scroll.y = bounds.y;
            scroll.w = scroll_w;
            scroll.h = bounds.h;  // TODO: Calculate

            RICO_prim_draw_rect(&border, &VEC4(0.2f, 0.2f, 0.2f, 0.5f));
            RICO_prim_draw_rect(&bounds, &VEC4(0.0f, 0.0f, 0.0f, 0.5f));
            RICO_prim_draw_rect(&scroll, &COLOR_ORANGE_HIGHLIGHT);

            RICO_heiro_render(sx, sy);

            RICO_prim_draw_rect(&cursor, &VEC4(1.0f, 0.0f, 0.0f, 0.5f));
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