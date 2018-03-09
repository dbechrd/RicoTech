#include "RICO/rico.h"

//int main_nuklear(int argc, char* argv[]);
//#include "3rdparty/main_nuke.c"

enum game_object_type
{
    OBJ_TIMMY = RICO_OBJECT_TYPE_COUNT,
    OBJ_GAME_PANEL,
    OBJ_GAME_BUTTON,
    OBJ_COUNT
};

enum actions
{
    ACTION_RICO_TEST = ACTION_COUNT,
    ACTION_TYPE_NEXT,
    ACTION_TYPE_PREV
};

struct timmy
{
    struct RICO_object rico;
    bool lights_on;
    bool audio_on;
};

struct game_button
{
    struct RICO_object rico;
    pkid panel_id;
    u32 index;
};

struct game_panel
{
    struct RICO_object rico;
    pkid buttons[9];
};

static struct game_panel panel_1;

static const char *PATH_ALPHA = "packs/alpha.pak";
static const char *PATH_ALPHA_SAV = "packs/alpha_sav.pak";

void pack_build_alpha()
{
    // TOOD: Refactor object data out into text files that can be read in and
    //       compiled into pack files. E.g. packs/alpha/objects/timmy.txt which
    //       references packs/alpha/textures/timmy.tga as "textures/timmy.tga"
    //       or just "timmy.tga".

	// TODO: Split objects (state) from resources (materials, textures, audio).
	//       pack_state: all world objects (for now)
	//       pack_alpha: textures, materials, audio, etc. specific to alpha

	u32 pack = RICO_pack_init(0, PATH_ALPHA, 64, MB(32));
	pkid tex_bricks_diff =
        RICO_load_texture_file(pack, "bricks_diff", "texture/cobble_diff.tga");
	pkid tex_bricks_mrao =
        RICO_load_texture_file(pack, "bricks_mrao", "texture/cobble_mrao.tga");
	pkid tex_bricks_emis =
        RICO_load_texture_color(pack, "bricks_emis", COLOR_TRANSPARENT);
	pkid mat_bricks =
        RICO_load_material(pack, "bricks", tex_bricks_diff, tex_bricks_mrao,
                           tex_bricks_emis);
    pkid mat_timmy = RICO_load_material(pack, "timmy", 0, 0, 0);

    DLB_ASSERT(tex_bricks_mrao);
    DLB_ASSERT(tex_bricks_diff);
    DLB_ASSERT(tex_bricks_emis);
    DLB_ASSERT(mat_bricks);

    pkid mesh_door_id;
    pkid mesh_terrain_id;
    pkid mesh_panel_id;
    pkid mesh_button_id;
    RICO_load_obj_file(pack, "mesh/alpha_terrain_001.obj", &mesh_terrain_id);
	RICO_load_obj_file(pack, "mesh/alpha_door_001.obj", &mesh_door_id);
    RICO_load_obj_file(pack, "mesh/alpha_staircase_001.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_wall_001.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_game_panel.obj", &mesh_panel_id);
	RICO_load_obj_file(pack, "mesh/alpha_game_button.obj", &mesh_button_id);

    DLB_ASSERT(mesh_door_id);
    DLB_ASSERT(mesh_terrain_id);
    DLB_ASSERT(mesh_panel_id);
    DLB_ASSERT(mesh_button_id);

    u32 pack_sav = RICO_pack_init(0, PATH_ALPHA_SAV, 64, MB(32));
    pkid terrain_id = RICO_load_object(pack_sav, RICO_OBJECT_TYPE_TERRAIN, 0,
                                       "terrain");
    struct RICO_object *terrain = RICO_pack_lookup(terrain_id);
    terrain->mesh_id = mesh_terrain_id;
    terrain->material_id = mat_bricks;

    pkid timmy_id = RICO_load_object(pack_sav, OBJ_TIMMY, sizeof(struct timmy),
                                     "timmy");
    struct timmy *timmy = RICO_pack_lookup(timmy_id);
    struct RICO_mesh *mesh_door = RICO_pack_lookup(mesh_door_id);
    timmy->rico.mesh_id = mesh_door_id;
    timmy->rico.material_id = mat_timmy;
    timmy->rico.bbox = mesh_door->bbox;
    timmy->lights_on = true;
    timmy->audio_on = true;

    pkid panel_id = RICO_load_object(pack_sav, OBJ_GAME_PANEL,
                                    sizeof(struct game_panel), "panel");
    struct game_panel *panel = RICO_pack_lookup(panel_id);
    struct RICO_mesh *mesh_panel = RICO_pack_lookup(mesh_panel_id);
    panel->rico.mesh_id = mesh_panel_id;
    panel->rico.bbox = mesh_panel->bbox;

    pkid button_id;
    struct game_button *button;
    struct RICO_mesh *mesh_button = RICO_pack_lookup(mesh_button_id);
    for (u32 i = 0; i < ARRAY_COUNT(panel->buttons); i++)
    {
        button_id = RICO_load_object(pack_sav, OBJ_GAME_BUTTON,
                                     sizeof(struct game_button), "button");
        button = RICO_pack_lookup(button_id);
        button->rico.mesh_id = mesh_button_id;
        button->rico.bbox = mesh_button->bbox;
        button->panel_id = panel_id;
        button->index = i;
        panel->buttons[i] = button_id;
    }

    RICO_pack_save(pack, 0, false);
    RICO_pack_free(pack);
    RICO_pack_save(pack_sav, PATH_ALPHA_SAV, false);
	RICO_pack_free(pack_sav);
}

void pack_build_all()
{
	pack_build_alpha();
}

int pack_load_all()
{
	enum RICO_error err;

    err = RICO_pack_load(PATH_ALPHA, 0);
    if (err) return err;
    err = RICO_pack_load(PATH_ALPHA_SAV, &RICO_pack_active);
    if (err) return err;

	return err;
}

void timmy_interact(struct timmy *timmy)
{
    timmy->lights_on = !timmy->lights_on;
    RICO_lighting_enabled = timmy->lights_on;

    timmy->audio_on = !timmy->audio_on;
    (timmy->audio_on) ? RICO_audio_unmute() : RICO_audio_mute();
}

struct RICO_audio_buffer audio_buffer_button;
struct RICO_audio_source audio_source_button;
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

    RICO_audio_source_play(&audio_source_button);
}

static pkid last_clicked;
void object_interact()
{
    pkid id = RICO_mouse_raycast();
    if (!id) return;
    
    struct RICO_object *obj = RICO_pack_lookup(id);
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

    RICO_bind_action(ACTION_RICO_TEST, CHORD_REPEAT1(SDL_SCANCODE_Z));
    RICO_bind_action(ACTION_TYPE_NEXT, CHORD1(SDL_SCANCODE_X));
    RICO_bind_action(ACTION_TYPE_PREV, CHORD1(SDL_SCANCODE_C));

    // TODO: Add audio to pack
    struct RICO_audio_buffer buffer;
    struct RICO_audio_source source;
    RICO_audio_buffer_load_file(&buffer, "audio/thunder_storm.ric");
    RICO_audio_source_init(&source);
    RICO_audio_source_buffer(&source, &buffer);
    RICO_audio_source_play_loop(&source);

    RICO_audio_buffer_load_file(&audio_buffer_button, "audio/bloop2.ric");
    RICO_audio_source_init(&audio_source_button);
    RICO_audio_source_buffer(&audio_source_button, &audio_buffer_button);

    while (!RICO_quit())
    {
        // TODO: Everything that happens here is delayed by a frame
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
                    RICO_audio_source_play(&audio_source_button);
                    break;
                default:
                    break;
            }
        }

        err = RICO_update();
        if (err) break;
    }

    RICO_cleanup();
    return err;
}
