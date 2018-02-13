#include "../../rico/src/rico_internal.h"
//#include "dlb_types.h"
//#define DLB_MATH_IMPLEMENTATION
//#include <math.h>
//#include "dlb_math.h"
//#include <stdlib.h>

//int main_nuklear(int argc, char* argv[]);
//#include "3rdparty/main_nuke.c"

struct pack *pack_active = 0;

void pack_build_alpha(u32 id)
{
	// TODO: Split objects (state) from resources (materials, textures, audio).
	//       pack_state: all world objects (for now)
	//       pack_alpha: textures, materials, audio, etc. specific to alpha
	const char *filename = "packs/alpha.pak";

	struct pack *pack = pack_init(id, filename, 128, MB(256));
	pkid bricks_diff = load_texture_file(pack, "Bricks_diff", "texture/cobble_diff.tga");
	pkid bricks_mrao = load_texture_file(pack, "Bricks_mrao", "texture/cobble_mrao.tga");
	pkid bricks_emis = load_texture_color(pack, "Bricks_emis", COLOR_TRANSPARENT);
	pkid bricks_mat = load_material(pack, "Bricks", bricks_diff, bricks_mrao, bricks_emis);

    RICO_ASSERT(bricks_diff);
    RICO_ASSERT(bricks_mrao);
    RICO_ASSERT(bricks_emis);
    RICO_ASSERT(bricks_mat);

	pkid door_mesh_pkid, ground_mesh_pkid;
	load_obj_file(pack, "mesh/alpha_door_001.obj", &door_mesh_pkid);
	load_obj_file(pack, "mesh/alpha_staircase_001.obj", 0);
	load_obj_file(pack, "mesh/alpha_wall_001.obj", 0);
	load_obj_file(pack, "mesh/alpha_terrain_001.obj", &ground_mesh_pkid);
	load_obj_file(pack, "mesh/alpha_game_panel.obj", 0);
	load_obj_file(pack, "mesh/alpha_game_button.obj", 0);

    RICO_ASSERT(door_mesh_pkid);
    RICO_ASSERT(ground_mesh_pkid);

    pkid ground_pkid = load_object(pack, OBJ_TERRAIN, "Ground");
    struct rico_object *ground = pack_lookup(ground_pkid);
    ground->props[PROP_MESH].type = PROP_MESH;
    ground->props[PROP_MESH].mesh_pkid = ground_mesh_pkid;
    ground->props[PROP_MATERIAL].type = PROP_MATERIAL;
    ground->props[PROP_MATERIAL].material_pkid = bricks_mat;

    pkid timmy_mat_pkid = load_material(pack, "Timmy", 0, 0, 0);
    pkid timmy_pkid = load_object(pack, OBJ_STATIC, "Timmy");
    struct rico_object *timmy = pack_lookup(timmy_pkid);
    struct rico_mesh *door_mesh = pack_lookup(door_mesh_pkid);
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

#if 0
	u32 timmy_mat = load_material(pack, "Timmy", 0, 0, 0);
	struct obj_property timmy_props[4] = { 0 };
	timmy_props[0].type = PROP_MESH;
	timmy_props[0].mesh_uid = door_mesh;
	timmy_props[1].type = PROP_MATERIAL;
	timmy_props[1].material_uid = timmy_mat;
	timmy_props[2].type = PROP_LIGHT_SWITCH;
	timmy_props[2].light_switch = (struct light_switch) { 3, true };
	timmy_props[3].type = PROP_AUDIO_SWITCH;
	timmy_props[3].audio_switch = (struct audio_switch) { 3, true };
	load_object(pack, "Timmy", OBJ_STATIC, ARRAY_COUNT(timmy_props),
				timmy_props, NULL);
#endif

	pack_save(pack, filename, false);
	pack_free(pack->id);
}

void pack_build_all()
{
	pack_build_alpha(packs_next);
	packs_next++;
}

int pack_load_all()
{
	enum rico_error err;

	err = pack_load("packs/alpha.pak", &pack_active);

	u32 count = ARRAY_COUNT(packs);
	for (u32 i = 0; i < count; ++i)
	{
		if (packs[i])
        {
			assert(i == packs[i]->id);
        }
	}

	return err;
}

int main(int argc, char **argv)
{
	//main_nuklear(argc, argv);
	RIC_init(argc, argv);

	pack_build_all();
	pack_load_all();

	RIC_run();
	RIC_quit();
	return 0;
}
