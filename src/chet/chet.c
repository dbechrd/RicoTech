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
    TOOLBAR_CURSOR,
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
    TOOLBAR_EXIT,
    TOOLBAR_COUNT,
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
    button->rico.material_id = button->rico.material_id == mat_on
        ? mat_off
        : mat_on;

    // Toggle neighbor buttons
    pkid button_id;
    struct game_button *other;
    // UP
    if (button->index >= 3)
    {
        button_id = panel->buttons[button->index - 3];
        other = RICO_pack_lookup(button_id);
        other->rico.material_id = other->rico.material_id == mat_on
            ? mat_off
            : mat_on;
    }
    // DOWN
    if (button->index < 6)
    {
        button_id = panel->buttons[button->index + 3];
        other = RICO_pack_lookup(button_id);
        other->rico.material_id = other->rico.material_id == mat_on
            ? mat_off
            : mat_on;
    }
    // LEFT
    if (button->index % 3 > 0)
    {
        button_id = panel->buttons[button->index - 1];
        other = RICO_pack_lookup(button_id);
        other->rico.material_id = other->rico.material_id == mat_on
            ? mat_off
            : mat_on;
    }
    // RIGHT
    if (button->index % 3 < 2)
    {
        button_id = panel->buttons[button->index + 1];
        other = RICO_pack_lookup(button_id);
        other->rico.material_id = other->rico.material_id == mat_on
            ? mat_off
            : mat_on;
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

static pkid last_clicked;
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

    // HACK: Display name of last-clicked object
    last_clicked = obj->uid.pkid;
    //RICO_load_string(PACK_TRANSIENT, 0, 0, 0, COLOR_WHITE, 0, 0, obj->uid.name);

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
                    RICO_object_trans_set(&obj->rico, &obj->rico.xform.position);
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
                    RICO_object_trans_set(&obj->rico, &obj->rico.xform.position);
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
        COLOR_DARK_WHITE,
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
                    RICO_prim_draw_sphere(&obj->rico.sphere, &COLOR_DARK_WHITE_HIGHLIGHT);
                    RICO_prim_draw_bbox(&obj->rico.bbox_world, &COLOR_DARK_WHITE_HIGHLIGHT);
                    RICO_prim_draw_obb(&obj->rico.obb, &COLOR_RED);
                }
                else if (obj->collide_aabb)
                {
                    RICO_prim_draw_sphere(&obj->rico.sphere, &COLOR_DARK_WHITE_HIGHLIGHT);
                    RICO_prim_draw_bbox(&obj->rico.bbox_world, &COLOR_ORANGE);
                    RICO_prim_draw_obb(&obj->rico.obb, &COLOR_DARK_WHITE_HIGHLIGHT);
                }
                else if (obj->collide_sphere)
                {
                    RICO_prim_draw_sphere(&obj->rico.sphere, &COLOR_YELLOW);
                    RICO_prim_draw_bbox(&obj->rico.bbox_world, &COLOR_DARK_WHITE_HIGHLIGHT);
                    RICO_prim_draw_obb(&obj->rico.obb, &COLOR_DARK_WHITE_HIGHLIGHT);
                }
                else
                {
                    RICO_prim_draw_sphere(&obj->rico.sphere, &COLOR_DARK_WHITE_HIGHLIGHT);
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
        toolbar_sprites[i].sheet = &toolbar_sheet;
        toolbar_sprites[i].coords = RECT(sprite_x, sprite_y, sprite_w, sprite_h);
        sprite_x += sprite_w;
        if (sprite_x >= tex_sheet->width)
        {
            sprite_x = 0;
            sprite_y += sprite_h;
        }
    }
}

static struct RICO_ui_element *hud;
static struct RICO_ui_element *buttons[ARRAY_COUNT(toolbar_sprites)];
static u32 pointless_buttons = 0;

void game_button_click(const struct RICO_ui_event_data *data)
{
    if (data->element == buttons[0] &&
        data->event_type == RICO_UI_EVENT_LMB_CLICK)
    {
        //data->element->color = COLOR_ORANGE;
        RICO_audio_source_play(&audio_sources[AUDIO_BUTTON]);
        pointless_buttons++;
    }
}

void game_render_ui()
{
    // HACK: Reset ui stack each frame
    // TODO: Reset this in the engine
    rico_ui_reset();

    hud = (struct RICO_ui_element *)RICO_ui_hud();
    hud->padding = RECT1(2);
    hud->color_default = COLOR_DARK_WHITE_HIGHLIGHT;

    for (int i = 0; i < ARRAY_COUNT(buttons); ++i)
    {
        struct RICO_ui_button *button = RICO_ui_button(hud);
        button->element.min_size = VEC2I(32, 32);
        button->element.margin = RECT1(2);
        button->element.color_default = COLOR_DARK_WHITE;
        button->element.color_hover = COLOR_ORANGE;
        button->element.color_click = COLOR_DARK_ORANGE;
        button->sprite = toolbar_sheet.sprites[i];
        button->element.event = game_button_click;
        buttons[i] = (struct RICO_ui_element *)button;
    }

    for (u32 i = 0; i < pointless_buttons; ++i)
    {
        struct RICO_ui_button *button = RICO_ui_button(hud);
        button->element.min_size = VEC2I(32, 32);
        button->element.margin = RECT1(2);
        button->element.color_default = COLOR_DARK_WHITE;
        button->element.color_hover = COLOR_ORANGE;
        button->element.color_click = COLOR_DARK_ORANGE;
        button->sprite = toolbar_sheet.sprites[0];
    }

    if (RICO_ui_layout(hud, 0, 0, 580, 0)) //mouse_x, mouse_y))
    {
        u32 start_x = (SCREEN_WIDTH / 2) - (hud->size.w / 2);
        RICO_ui_draw(hud, start_x, 20);
    }
    else
    {
        struct rect x_rect = { 16, 16, 32, 32 };
        float x = SCREEN_X(x_rect.x);
        float y = SCREEN_Y(x_rect.y);
        float w = SCREEN_W(x_rect.w);
        float h = SCREEN_H(x_rect.h);
        RICO_prim_draw_line2d(x, y, x + w, y + h, &COLOR_ORANGE);
        RICO_prim_draw_line2d(x + w, y, x, y + h, &COLOR_ORANGE);
    }

    ui_debug_stack_usage();
    RICO_ui_layout(&ui_debug_usage_hud->element, 300, 60, 0, 0);
    u32 ui_debug_stack_x = (SCREEN_WIDTH / 2) - (ui_debug_usage_hud->element.size.w / 2);
    RICO_ui_draw(&ui_debug_usage_hud->element, ui_debug_stack_x, 2);

    //struct rect cursor_rect = { mouse_x - 16, mouse_y - 16, 32, 32 };
    //struct rect cursor_rect = { mouse_x, mouse_y, 32, 32 };
    //RICO_prim_draw_rect_tex(&cursor_rect, &COLOR_TRANSPARENT, tex_toolbar);
    //RICO_prim_draw_rect(&RECT(mouse_x - 1, mouse_y - 1, 1, 1), &COLOR_RED);
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

    cereal_test();

    RICO_bind_action(ACTION_RICO_TEST, CHORD_REPEAT1(SDL_SCANCODE_Z));
    RICO_bind_action(ACTION_TYPE_NEXT, CHORD1(SDL_SCANCODE_X));
    RICO_bind_action(ACTION_TYPE_PREV, CHORD1(SDL_SCANCODE_C));

    RICO_audio_volume_set(0.1f);

    // TODO: Add audio to pack
    RICO_audio_buffer_load_file(&audio_buffers[AUDIO_WELCOME], "audio/welcome.ric");
    RICO_audio_source_init(&audio_sources[AUDIO_WELCOME]);
    RICO_audio_source_buffer(&audio_sources[AUDIO_WELCOME], &audio_buffers[AUDIO_WELCOME]);
    RICO_audio_source_play(&audio_sources[AUDIO_WELCOME]);

    RICO_audio_buffer_load_file(&audio_buffers[AUDIO_THUNDER], "audio/thunder_storm.ric");
    RICO_audio_source_init(&audio_sources[AUDIO_THUNDER]);
    RICO_audio_source_buffer(&audio_sources[AUDIO_THUNDER], &audio_buffers[AUDIO_THUNDER]);
    RICO_audio_source_play_loop(&audio_sources[AUDIO_THUNDER]);

    RICO_audio_buffer_load_file(&audio_buffers[AUDIO_BUTTON], "audio/bloop2.ric");
    RICO_audio_source_init(&audio_sources[AUDIO_BUTTON]);
    RICO_audio_source_buffer(&audio_sources[AUDIO_BUTTON], &audio_buffers[AUDIO_BUTTON]);

    RICO_audio_buffer_load_file(&audio_buffers[AUDIO_VICTORY], "audio/victory.ric");
    RICO_audio_source_init(&audio_sources[AUDIO_VICTORY]);
    RICO_audio_source_buffer(&audio_sources[AUDIO_VICTORY], &audio_buffers[AUDIO_VICTORY]);

    game_toolbar_init();

    // HACK: Find Timmy by name and use light/audio flags to determine start-up
    //       state of lighting and audio.
    struct timmy *timmy =
        RICO_pack_lookup_by_name(pack_table[PACK_ALPHA].sav_id, "timmy");
    DLB_ASSERT(timmy);
    timmy_state_hacks(timmy->lights_on, timmy->audio_on);

    while (!RICO_quit())
    {
        RICO_mouse_coords(&mouse_x, &mouse_y);

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

        if (!RICO_simulation_paused())
        {
            clash_simulate(timmy);
        }

        err = RICO_update();
        if (err) break;

        //clash_detect(timmy);

#if 0
        RICO_render();
#else
        RICO_render_objects();
        RICO_render_editor();
        DEBUG_render_bboxes(timmy);
        if (rayviz_sphere.radius > 0.0f)
            RICO_prim_draw_sphere(&rayviz_sphere, &COLOR_YELLOW);
        game_render_ui();
        RICO_render_ui();

        RICO_frame_swap();
#endif
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