#include "chet.h"
#include "chet_packs.h"

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

#define PACK_ALPHA 0
#define PACK_CLASH 1

static struct pack_info pack_table[] =
{
    { "packs/alpha.pak" , "packs/alpha.sav", 0 },
    { "packs/clash.pak" , "packs/clash.sav", 0 }
};

//-------------------------------------------------------------------

static const float GRAVITY = -0.0098f; // TODO: Scale by delta_time properly
static const float COEF_COLLIDE = 0.15f; // TODO: Elastic collision coef

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
static pkid mat_rayviz_id;
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
    //pos.z += scale / 2.0f;

#if 0
    pkid rayviz_id = RICO_load_object(RICO_pack_active, OBJ_RAY_VISUALIZER,
                                      sizeof(struct ray_visualizer), "Ray Viz");
    struct ray_visualizer *rayviz = RICO_pack_lookup(rayviz_id);
    rayviz->lifetime = 30;  // TODO: Add these objects to a temp buffer, then
                            //       scan it each frame to remove dead object.
    rayviz->rico.material_id = mat_rayviz_id;
    rayviz->rico.xform.scale = VEC3(scale, scale, scale);
    RICO_object_trans_set(&rayviz->rico, &pos);
#else
    rayviz_sphere.orig = pos;
    rayviz_sphere.radius = scale;
#endif

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

void clash_simulate()
{
    struct small_cube *last = RICO_pack_last(pack_table[PACK_CLASH].sav_id,
                                             RICO_HND_OBJECT);
    struct small_cube *obj = RICO_pack_first(pack_table[PACK_CLASH].sav_id,
                                             RICO_HND_OBJECT);
    for (;;)
    {
        DLB_ASSERT(obj->rico.type == OBJ_SMALL_CUBE);

        if (!v3_equals(&obj->acc, &VEC3_ZERO))
        {
            v3_add(&obj->vel, &obj->acc);
            // TODO: Drag coef

            // TODO: Collision detection
            if (obj->rico.xform.position.y + obj->vel.y <= 0.0f)
            {
                if (obj->vel.y < 0.0f)
                    obj->vel.y *= -1.0f;
                obj->vel.y *= COEF_COLLIDE;

                // TODO: Epsilon (must be bigger than GRAVITY * friction)
                if (fabs(obj->vel.y) < 0.01f)
                {
                    obj->acc = VEC3_ZERO;
                    obj->vel = VEC3_ZERO;
                    obj->rico.xform.position.y = 0.0f;
                }
            }
            else
            {
                v3_add(&obj->rico.xform.position, &obj->vel);
            }

            RICO_object_trans_set(&obj->rico, &obj->rico.xform.position);
        }
        
        if (obj == last)
            break;
        obj = RICO_pack_next(obj->rico.uid.pkid);
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

    RICO_bind_action(ACTION_RICO_TEST, CHORD_REPEAT1(SDL_SCANCODE_Z));
    RICO_bind_action(ACTION_TYPE_NEXT, CHORD1(SDL_SCANCODE_X));
    RICO_bind_action(ACTION_TYPE_PREV, CHORD1(SDL_SCANCODE_C));

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

    pkid tex_rayviz_id = RICO_load_texture_color(RICO_pack_active, "rayviz", COLOR_RED);
    mat_rayviz_id = RICO_load_material(RICO_pack_active, "rayviz", tex_rayviz_id, 0, 0);

    // HACK: Find Timmy by name and use light/audio flags to determine start-up
    //       state of lighting and audio.
    struct timmy *timmy =
        RICO_pack_lookup_by_name(pack_table[PACK_ALPHA].sav_id, "timmy");
    DLB_ASSERT(timmy);
    timmy_state_hacks(timmy->lights_on, timmy->audio_on);

    while (!RICO_quit())
    {
        u32 action;
        while (RICO_key_event(&action))
        {
            if (RICO_state_is_edit() || RICO_state_is_paused())
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

        clash_simulate();

        err = RICO_update();
        if (err) break;

#if 0
        RICO_render();
#else
        RICO_render_objects();
        RICO_render_editor();
        if (rayviz_sphere.radius > 0.0f)
            RICO_prim_draw_sphere(&rayviz_sphere, &COLOR_YELLOW);
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
