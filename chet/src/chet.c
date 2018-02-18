#include "RICO/rico.h"

//int main_nuklear(int argc, char* argv[]);
//#include "3rdparty/main_nuke.c"

struct pack *pack_active = 0;

void pack_build_alpha(u32 id)
{
	// TODO: Split objects (state) from resources (materials, textures, audio).
	//       pack_state: all world objects (for now)
	//       pack_alpha: textures, materials, audio, etc. specific to alpha
	const char *filename = "packs/alpha.pak";

	struct pack *pack = RICO_pack_init(id, filename, 128, MB(256));
	pkid bricks_diff = RICO_load_texture_file(pack, "Bricks_diff", "texture/cobble_diff.tga");
	pkid bricks_mrao = RICO_load_texture_file(pack, "Bricks_mrao", "texture/cobble_mrao.tga");
	pkid bricks_emis = RICO_load_texture_color(pack, "Bricks_emis", COLOR_TRANSPARENT);
	pkid bricks_mat = RICO_load_material(pack, "Bricks", bricks_diff, bricks_mrao, bricks_emis);

    assert(bricks_mrao);
    assert(bricks_diff);
    assert(bricks_emis);
    assert(bricks_mat);

	pkid door_mesh_pkid, ground_mesh_pkid;
	RICO_load_obj_file(pack, "mesh/alpha_door_001.obj", &door_mesh_pkid);
	RICO_load_obj_file(pack, "mesh/alpha_staircase_001.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_wall_001.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_terrain_001.obj", &ground_mesh_pkid);
	RICO_load_obj_file(pack, "mesh/alpha_game_panel.obj", 0);
	RICO_load_obj_file(pack, "mesh/alpha_game_button.obj", 0);

    assert(door_mesh_pkid);
    assert(ground_mesh_pkid);

    pkid ground_pkid = RICO_load_object(pack, OBJ_TERRAIN, "Ground");
    struct rico_object *ground = RICO_pack_lookup(ground_pkid);
    ground->props[PROP_MESH].type = PROP_MESH;
    ground->props[PROP_MESH].mesh_pkid = ground_mesh_pkid;
    ground->props[PROP_MATERIAL].type = PROP_MATERIAL;
    ground->props[PROP_MATERIAL].material_pkid = bricks_mat;

    pkid timmy_mat_pkid = RICO_load_material(pack, "Timmy", 0, 0, 0);
    pkid timmy_pkid = RICO_load_object(pack, OBJ_STATIC, "Timmy");
    struct rico_object *timmy = RICO_pack_lookup(timmy_pkid);
    struct rico_mesh *door_mesh = RICO_pack_lookup(door_mesh_pkid);
    timmy->props[PROP_MESH].type = PROP_MESH;
    timmy->props[PROP_MESH].mesh_pkid = door_mesh_pkid;
    timmy->props[PROP_MATERIAL].type = PROP_MATERIAL;
    timmy->props[PROP_MATERIAL].material_pkid = timmy_mat_pkid;
    timmy->props[PROP_BBOX].type = PROP_BBOX;
    timmy->props[PROP_BBOX].bbox = door_mesh->bbox;
    timmy->props[PROP_LIGHT_SWITCH].type = PROP_LIGHT_SWITCH;
    timmy->props[PROP_LIGHT_SWITCH].light_switch =
        (struct light_switch) { 3, true };
    timmy->props[PROP_AUDIO_SWITCH].type = PROP_AUDIO_SWITCH;
    timmy->props[PROP_AUDIO_SWITCH].audio_switch =
        (struct audio_switch) { 3, true };

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

// HACK: I want to make a light switch!
internal void object_interact_light_switch(struct rico_object *obj)
{
    UNUSED(obj);
    RICO_lighting_enabled = !RICO_lighting_enabled;
}

// HACK: I want to make an audio switch!
internal void object_interact_audio_switch(struct rico_object *obj)
{
    UNUSED(obj);
    static audio_play = true;
    
    audio_play = !audio_play;
    (audio_play) ? RICO_audio_unmute() : RICO_audio_mute();
}

internal void object_interact_game_button(struct rico_object *obj)
{
    struct obj_property *prop = &obj->props[PROP_GAME_BUTTON];
    assert(prop->type);
    prop->game_button.state = !prop->game_button.state;
}

RICO_EVENT_OBJECT(object_interact)
{
    for (u32 i = 0; i < PROP_COUNT; ++i)
    {
        switch (obj->props[i].type)
        {
            case PROP_LIGHT_SWITCH:
                object_interact_light_switch(obj);
                break;
            case PROP_AUDIO_SWITCH:
                object_interact_audio_switch(obj);
                break;
            case PROP_GAME_BUTTON:
                object_interact_game_button(obj);
                break;
        }
    }
}

struct rico_audio_buffer buffer;
struct rico_audio_source source;

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

	//main_nuklear(argc, argv);
    RICO_init();;
	pack_build_all();
	pack_load_all();

    RICO_event_object_interact = &object_interact;

    RICO_audio_buffer_load_file(&buffer, "audio/thunder_storm.raw");
    RICO_audio_source_init(&source);
    RICO_audio_source_buffer(&source, &buffer);
    RICO_audio_source_play_loop(&source);

	RICO_run();
	RICO_quit();
	return 0;
}
