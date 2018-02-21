#include "RICO/rico.h"

//int main_nuklear(int argc, char* argv[]);
//#include "3rdparty/main_nuke.c"

struct pack *pack_active = 0;

enum game_object_type
{
    OBJ_TIMMY = RICO_OBJECT_TYPE_START,
    OBJ_ETC
};

RICO_OBJECT(timmy)
    u32 light_id;
    u32 audio_id;
    bool state;
};

//struct timmy
//{
//    struct rico_object rico;
//    u32 light_id;
//    u32 audio_id;
//    bool state;
//};

void pack_build_alpha(u32 id)
{
    // TOOD: Refactor object data out into text files that can be read in and
    //       compiled into pack files. E.g. packs/alpha/objects/timmy.txt which
    //       references packs/alpha/textures/timmy.tga as "textures/timmy.tga"
    //       or just "timmy.tga".

	// TODO: Split objects (state) from resources (materials, textures, audio).
	//       pack_state: all world objects (for now)
	//       pack_alpha: textures, materials, audio, etc. specific to alpha
	const char *filename = "packs/alpha.pak";

	struct pack *pack = RICO_pack_init(id, filename, 128, MB(256));
	pkid bricks_diff = RICO_load_texture_file(pack, "Bricks_diff", "texture/cobble_diff.tga");
	pkid bricks_mrao = RICO_load_texture_file(pack, "Bricks_mrao", "texture/cobble_mrao.tga");
	pkid bricks_emis = RICO_load_texture_color(pack, "Bricks_emis", COLOR_TRANSPARENT);
	pkid bricks_mat = RICO_load_material(pack, "Bricks", bricks_diff, bricks_mrao, bricks_emis);

    DLB_ASSERT(bricks_mrao);
    DLB_ASSERT(bricks_diff);
    DLB_ASSERT(bricks_emis);
    DLB_ASSERT(bricks_mat);

	pkid door_mesh_pkid, ground_mesh_pkid;
	RICO_load_obj_file(pack, "mesh/alpha_door_001.obj", &door_mesh_pkid);
	RICO_load_obj_file(pack, "mesh/alpha_staircase_001.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_wall_001.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_terrain_001.obj", &ground_mesh_pkid);
	RICO_load_obj_file(pack, "mesh/alpha_game_panel.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_game_button.obj", 0);

    DLB_ASSERT(door_mesh_pkid);
    DLB_ASSERT(ground_mesh_pkid);

    pkid ground_pkid = RICO_load_object(pack, RICO_OBJECT_TYPE_TERRAIN, 0,
                                        "Ground");
    struct rico_object *ground = RICO_pack_lookup(ground_pkid);
    ground->mesh_pkid = ground_mesh_pkid;
    ground->material_pkid = bricks_mat;

    pkid timmy_mat_pkid = RICO_load_material(pack, "Timmy", 0, 0, 0);
    pkid timmy_pkid = RICO_load_object(pack, OBJ_TIMMY, sizeof(struct timmy),
                                       "Timmy");
    struct timmy *timmy = RICO_pack_lookup(timmy_pkid);
    struct rico_mesh *door_mesh = RICO_pack_lookup(door_mesh_pkid);
    timmy->rico.mesh_pkid = door_mesh_pkid;
    timmy->rico.material_pkid = timmy_mat_pkid;
    timmy->rico.bbox = door_mesh->bbox;
    timmy->audio_id = 19;
    timmy->light_id = 68;
    // TODO: How do I implement this...?
    //timmy->light_switch =
    //    (struct light_switch) { 3, true };
    //timmy->audio_switch =
    //    (struct audio_switch) { 3, true };

	RICO_pack_save(pack, filename, false);
	RICO_pack_free(pack->id);
}

void pack_build_all()
{
	pack_build_alpha(RICO_packs_next);
	RICO_packs_next++;
}

int pack_load_all()
{
	enum rico_error err;

	err = RICO_pack_load("packs/alpha.pak", &pack_active);

	for (u32 i = 0; i < ARRAY_COUNT(RICO_packs); ++i)
	{
		if (RICO_packs[i])
        {
			assert(i == RICO_packs[i]->id);
        }
	}

	return err;
}

internal void timmy_interact(struct timmy *obj)
{
    UNUSED(obj);
    RICO_lighting_enabled = !RICO_lighting_enabled;

    static bool audio_play = true;
    audio_play = !audio_play;
    (audio_play) ? RICO_audio_unmute() : RICO_audio_mute();
}

RICO_EVENT_OBJECT(object_interact)
{
    switch (obj->type)
    {
        case OBJ_TIMMY:
            timmy_interact((struct timmy *)obj);
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

struct rico_audio_buffer buffer;
struct rico_audio_source source;

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

	//main_nuklear(argc, argv);
    RICO_init();
	//pack_build_all();
	pack_load_all();

    RICO_event_object_interact = &object_interact;

    // TODO: Add audio to pack
    RICO_audio_buffer_load_file(&buffer, "audio/thunder_storm.raw");
    RICO_audio_source_init(&source);
    RICO_audio_source_buffer(&source, &buffer);
    RICO_audio_source_play_loop(&source);

	RICO_run();
	RICO_quit();
	return 0;
}
