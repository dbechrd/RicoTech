#include "chet_packs.h"

void pack_build_alpha(struct pack_info **pack_table, const char *filename)
{
    PERF_START(pack_build);

    // TOOD: Refactor object data out into text files that can be read in and
    //       compiled into pack files. E.g. packs/alpha/objects/timmy.txt which
    //       references packs/alpha/textures/timmy.tga as "textures/timmy.tga"
    //       or just "timmy.tga".

    // TODO: Split objects (state) from resources (materials, textures, audio).
    //       pack_state: all world objects (for now)
    //       pack_alpha: textures, materials, audio, etc. specific to alpha
    struct pack_info *entry = dlb_vec_alloc(*pack_table);
    entry->path = filename;
    entry->id = ric_pack_init(0, entry->path, 64, MB(128));

    pkid tex_bricks_diff = ric_load_texture_file(entry->id, "bricks_diff", "texture/cobble_diff.tga");
    pkid tex_bricks_mrao = ric_load_texture_file(entry->id, "bricks_mrao", "texture/cobble_mrao.tga");
    pkid tex_bricks_emis = ric_load_texture_color(entry->id, "bricks_emis", &COLOR_TRANSPARENT);
    pkid mat_bricks = ric_load_material(entry->id, "bricks", tex_bricks_diff, tex_bricks_mrao, tex_bricks_emis);
    pkid mat_timmy = ric_load_material(entry->id, "mat_timmy", 0, 0, 0);

    DLB_ASSERT(tex_bricks_mrao);
    DLB_ASSERT(tex_bricks_diff);
    DLB_ASSERT(tex_bricks_emis);
    DLB_ASSERT(mat_bricks);
    DLB_ASSERT(mat_timmy);

    pkid tex_sky_posx = ric_load_texture_file(entry->id, "sky_posx", "texture/sky_posx.tga");
    pkid tex_sky_negx = ric_load_texture_file(entry->id, "sky_negx", "texture/sky_negx.tga");
    pkid tex_sky_posy = ric_load_texture_file(entry->id, "sky_posy", "texture/sky_posy.tga");
    pkid tex_sky_negy = ric_load_texture_file(entry->id, "sky_negy", "texture/sky_negy.tga");
    pkid tex_sky_posz = ric_load_texture_file(entry->id, "sky_posz", "texture/sky_posz.tga");
    pkid tex_sky_negz = ric_load_texture_file(entry->id, "sky_negz", "texture/sky_negz.tga");

    pkid mesh_terrain_id;
    pkid mesh_door_id;
    pkid mesh_panel_id;
    pkid mesh_button_id;
    // TODO: Associate shader program with material, not mesh (how do others handle vertex layouts?)
    ric_load_obj_file(entry->id, "mesh/alpha_terrain_001.obj", &mesh_terrain_id, PROG_PBR);
    ric_load_obj_file(entry->id, "mesh/alpha_door_001.obj", &mesh_door_id, PROG_PBR);
    ric_load_obj_file(entry->id, "mesh/alpha_game_panel.obj", &mesh_panel_id, PROG_PBR);
    ric_load_obj_file(entry->id, "mesh/alpha_game_button.obj", &mesh_button_id, PROG_PBR);
    ric_load_obj_file(entry->id, "mesh/alpha_staircase_001.obj", 0, PROG_PBR);
    ric_load_obj_file(entry->id, "mesh/alpha_wall_001.obj", 0, PROG_PBR);
    ric_load_obj_file(entry->id, "mesh/alpha_room.obj", 0, PROG_PBR);

    DLB_ASSERT(mesh_door_id);
    DLB_ASSERT(mesh_terrain_id);
    DLB_ASSERT(mesh_panel_id);
    DLB_ASSERT(mesh_button_id);

    pkid terrain_id = ric_load_object(entry->id, OBJ_TERRAIN, 0, "terrain");
    struct ric_object *terrain = ric_pack_lookup(terrain_id);
    ric_object_mesh_set(terrain, mesh_terrain_id);
    ric_object_material_set(terrain, mat_bricks);
    terrain->select_ignore = true;

    pkid timmy_id = ric_load_object(entry->id, OBJ_TIMMY, sizeof(struct timmy), "timmy");
    struct timmy *timmy = ric_pack_lookup(timmy_id);
    ric_object_trans_set(&timmy->rico, &VEC3(-4.0f, 0.0f, 0.0f));
    ric_object_mesh_set(&timmy->rico, mesh_door_id);
    ric_object_material_set(&timmy->rico, mat_timmy);
    timmy->lights_on = true;
    timmy->audio_on = true;

    pkid light_test_id = ric_load_object(entry->id, OBJ_LIGHT_TEST, sizeof(struct light_test), "Test light");
    struct light_test *light_test = ric_pack_lookup(light_test_id);
    ric_object_trans_set(&light_test->rico, &VEC3(0.0f, 4.0f, 0.0f));

    pkid lighting_test_cube_id = ric_load_object(entry->id, OBJ_LIGHT_TEST_CUBE, sizeof(struct ric_object), "Lighting test cube");
    struct ric_object *lighting_test_cube = ric_pack_lookup(lighting_test_cube_id);
    ric_object_trans_set(lighting_test_cube, &VEC3(0.0f, 2.0f, 0.0f));

    /*
    pkid panel_id = ric_load_object(entry->id, OBJ_GAME_PANEL,
                                     sizeof(struct game_panel), "panel");
    struct game_panel *panel = ric_pack_lookup(panel_id);
    ric_object_mesh_set(&panel->rico, mesh_panel_id);

    pkid button_id;
    struct game_button *button;
    for (u32 i = 0; i < ARRAY_COUNT(panel->buttons); i++)
    {
        button_id = ric_load_object(entry->id, OBJ_GAME_BUTTON,
                                     sizeof(struct game_button), "button");
        button = ric_pack_lookup(button_id);
        ric_object_mesh_set(&button->rico, mesh_button_id);
        button->panel_id = panel_id;
        button->index = i;
        panel->buttons[i] = button_id;
    }
    */

    ric_pack_save(entry->id, false);
    ric_pack_free(entry->id);

    PERF_END_MSG(pack_build, "Built pack '%s'\n", filename);
}