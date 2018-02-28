#include "RICO/rico.h"

//int main_nuklear(int argc, char* argv[]);
//#include "3rdparty/main_nuke.c"

enum game_object_type
{
    OBJ_TIMMY = RICO_OBJECT_TYPE_COUNT,
    OBJ_GAME_PANEL,
    OBJ_GAME_BUTTON
};

enum actions
{
    ACTION_RICO_TEST = ACTION_COUNT
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
};

struct game_panel
{
    struct RICO_object rico;
    struct game_button buttons[9];
};

static struct game_panel panel_1;

static const char *PATH_ALPHA = "packs/alpha.pak";
//static const char *PATH_ALPHA_SAV = "packs/alpha_sav.pak";

void pack_build_alpha(u32 id)
{
    // TOOD: Refactor object data out into text files that can be read in and
    //       compiled into pack files. E.g. packs/alpha/objects/timmy.txt which
    //       references packs/alpha/textures/timmy.tga as "textures/timmy.tga"
    //       or just "timmy.tga".

	// TODO: Split objects (state) from resources (materials, textures, audio).
	//       pack_state: all world objects (for now)
	//       pack_alpha: textures, materials, audio, etc. specific to alpha

	struct pack *pack = RICO_pack_init(id, PATH_ALPHA, 64, MB(32));
	pkid tex_bricks_diff = RICO_load_texture_file(pack, "bricks_diff", "texture/cobble_diff.tga");
	pkid tex_bricks_mrao = RICO_load_texture_file(pack, "bricks_mrao", "texture/cobble_mrao.tga");
	pkid tex_bricks_emis = RICO_load_texture_color(pack, "bricks_emis", COLOR_TRANSPARENT);
	pkid mat_bricks = RICO_load_material(pack, "bricks", tex_bricks_diff, tex_bricks_mrao, tex_bricks_emis);
    pkid mat_timmy = RICO_load_material(pack, "timmy", 0, 0, 0);

    DLB_ASSERT(tex_bricks_mrao);
    DLB_ASSERT(tex_bricks_diff);
    DLB_ASSERT(tex_bricks_emis);
    DLB_ASSERT(mat_bricks);

	pkid mesh_door, mesh_terrain;
    RICO_load_obj_file(pack, "mesh/alpha_terrain_001.obj", &mesh_terrain);
	RICO_load_obj_file(pack, "mesh/alpha_door_001.obj", &mesh_door);
    RICO_load_obj_file(pack, "mesh/alpha_staircase_001.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_wall_001.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_game_panel.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_game_button.obj", 0);

    DLB_ASSERT(mesh_door);
    DLB_ASSERT(mesh_terrain);

    //struct pack *pack_sav = RICO_pack_init(id + 1, PATH_ALPHA_SAV, 512, MB(32));
    pkid terrain_id = RICO_load_object(pack, RICO_OBJECT_TYPE_TERRAIN, 0, "terrain");
    struct RICO_object *terrain = RICO_pack_lookup(terrain_id);
    terrain->mesh_id = mesh_terrain;
    terrain->material_id = mat_bricks;

    pkid timmy_id = RICO_load_object(pack, OBJ_TIMMY, sizeof(struct timmy), "timmy");
    struct timmy *timmy = RICO_pack_lookup(timmy_id);
    struct RICO_mesh *door_mesh = RICO_pack_lookup(mesh_door);
    timmy->rico.mesh_id = mesh_door;
    timmy->rico.material_id = mat_timmy;
    timmy->rico.bbox = door_mesh->bbox;
    timmy->lights_on = true;
    timmy->audio_on = true;

    RICO_load_object(pack, OBJ_GAME_BUTTON, sizeof(struct game_button), "button");

	RICO_pack_save(pack, PATH_ALPHA, false);
    //RICO_pack_save(pack_sav, PATH_ALPHA_SAV, false);
    RICO_pack_free(pack->id);
	//RICO_pack_free(pack_sav->id);
}

void pack_build_all()
{
	pack_build_alpha(RICO_packs_next);
	RICO_packs_next++;
}

int pack_load_all()
{
	enum RICO_error err;

    err = RICO_pack_load(PATH_ALPHA, &pack_active);
    //err = RICO_pack_load(PATH_ALPHA_SAV, &pack_active);

	for (u32 i = 0; i < ARRAY_COUNT(RICO_packs); ++i)
	{
		if (RICO_packs[i])
        {
			assert(i == RICO_packs[i]->id);
        }
	}

	return err;
}

void timmy_interact(struct timmy *obj)
{
    obj->lights_on = !obj->lights_on;
    RICO_lighting_enabled = obj->lights_on;

    obj->audio_on = !obj->audio_on;
    (obj->audio_on) ? RICO_audio_unmute() : RICO_audio_mute();
}

struct RICO_audio_buffer audio_buffer_button;
struct RICO_audio_source audio_source_button;
void game_button_interact(struct game_button *obj)
{
    UNUSED(obj);
    RICO_audio_source_play(&audio_source_button);
}

void object_interact(struct RICO_object *obj)
{
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
	pack_build_all();
	pack_load_all();

    RICO_event_object_interact = &object_interact;

    RICO_bind_action(ACTION_RICO_TEST, CHORD1(SDL_SCANCODE_Z));

    // TODO: Add audio to pack
    struct RICO_audio_buffer buffer;
    struct RICO_audio_source source;
    RICO_audio_buffer_load_file(&buffer, "audio/thunder_storm.raw");
    RICO_audio_source_init(&source);
    RICO_audio_source_buffer(&source, &buffer);
    RICO_audio_source_play_loop(&source);

    RICO_audio_buffer_load_file(&audio_buffer_button, "audio/door_bang.raw");
    RICO_audio_source_init(&audio_source_button);
    RICO_audio_source_buffer(&audio_source_button, &audio_buffer_button);

    while (!RICO_quit())
    {
        err = RICO_update();
        if (err) break;

        // TODO: Everything that happens here is delayed by a frame
        enum RICO_action action = RICO_key_event();
        while (action)
        {
            switch (action)
            {
                case ACTION_RICO_TEST:
                    RICO_audio_source_play(&audio_source_button);
                    break;
                default:
                    break;
            }
            action = RICO_key_event();
        }
    }

    RICO_cleanup();
    return err;
}
