#include "chet_packs.h"

void pack_build_alpha(struct pack_info *pack_info)
{
    // TOOD: Refactor object data out into text files that can be read in and
    //       compiled into pack files. E.g. packs/alpha/objects/timmy.txt which
    //       references packs/alpha/textures/timmy.tga as "textures/timmy.tga"
    //       or just "timmy.tga".

    // TODO: Split objects (state) from resources (materials, textures, audio).
    //       pack_state: all world objects (for now)
    //       pack_alpha: textures, materials, audio, etc. specific to alpha
    u32 pack_dat = ric_pack_init(0, pack_info->path_pak, 64, MB(128));

    pkid tex_bricks_diff = ric_load_texture_file(
        pack_dat, "bricks_diff", "texture/cobble_diff.tga");
    pkid tex_bricks_mrao = ric_load_texture_file(
        pack_dat, "bricks_mrao", "texture/cobble_mrao.tga");
    pkid tex_bricks_emis = ric_load_texture_color(
        pack_dat, "bricks_emis", &COLOR_TRANSPARENT);
    pkid mat_bricks = ric_load_material(
        pack_dat, "bricks", tex_bricks_diff, tex_bricks_mrao, tex_bricks_emis);
    pkid mat_timmy = ric_load_material(pack_dat, "timmy", 0, 0, 0);

    DLB_ASSERT(tex_bricks_mrao);
    DLB_ASSERT(tex_bricks_diff);
    DLB_ASSERT(tex_bricks_emis);
    DLB_ASSERT(mat_bricks);
    DLB_ASSERT(mat_timmy);

    pkid tex_sky_posx = ric_load_texture_file(
        pack_dat, "sky_posx", "texture/sky_posx.tga");
    pkid tex_sky_negx = ric_load_texture_file(
        pack_dat, "sky_negx", "texture/sky_negx.tga");
    pkid tex_sky_posy = ric_load_texture_file(
        pack_dat, "sky_posy", "texture/sky_posy.tga");
    pkid tex_sky_negy = ric_load_texture_file(
        pack_dat, "sky_negy", "texture/sky_negy.tga");
    pkid tex_sky_posz = ric_load_texture_file(
        pack_dat, "sky_posz", "texture/sky_posz.tga");
    pkid tex_sky_negz = ric_load_texture_file(
        pack_dat, "sky_negz", "texture/sky_negz.tga");

    pkid mesh_terrain_id;
    pkid mesh_door_id;
    pkid mesh_panel_id;
    pkid mesh_button_id;
    ric_load_obj_file(
        pack_dat, "mesh/alpha_terrain_001.obj", &mesh_terrain_id, PROG_PBR);
    ric_load_obj_file(
        pack_dat, "mesh/alpha_door_001.obj", &mesh_door_id, PROG_PBR);
    ric_load_obj_file(
        pack_dat, "mesh/alpha_game_panel.obj", &mesh_panel_id, PROG_PBR);
    ric_load_obj_file(
        pack_dat, "mesh/alpha_game_button.obj", &mesh_button_id, PROG_PBR);
    ric_load_obj_file(
        pack_dat, "mesh/alpha_staircase_001.obj", 0, PROG_PBR);
    ric_load_obj_file(
        pack_dat, "mesh/alpha_wall_001.obj", 0, PROG_PBR);
    ric_load_obj_file(
        pack_dat, "mesh/alpha_room.obj", 0, PROG_PBR);

    DLB_ASSERT(mesh_door_id);
    DLB_ASSERT(mesh_terrain_id);
    DLB_ASSERT(mesh_panel_id);
    DLB_ASSERT(mesh_button_id);

    u32 pack_sav = ric_pack_init(0, pack_info->path_sav, 64, MB(32));
    pkid terrain_id = ric_load_object(pack_sav, OBJ_TERRAIN, 0, "terrain");
    struct ric_object *terrain = ric_pack_lookup(terrain_id);
    ric_object_mesh_set(terrain, mesh_terrain_id);
    ric_object_material_set(terrain, mat_bricks);
    terrain->select_ignore = true;

    pkid timmy_id = ric_load_object(
        pack_sav, OBJ_TIMMY, sizeof(struct timmy), "timmy");
    struct timmy *timmy = ric_pack_lookup(timmy_id);
    ric_object_trans_set(&timmy->rico, &VEC3(-4.0f, 0.0f, 0.0f));
    ric_object_mesh_set(&timmy->rico, mesh_door_id);
    ric_object_material_set(&timmy->rico, mat_timmy);
    timmy->lights_on = true;
    timmy->audio_on = true;

    pkid light_test_id = ric_load_object(
        pack_sav, OBJ_LIGHT_TEST, sizeof(struct light_test), "Test light");
    struct light_test *light_test = ric_pack_lookup(light_test_id);
    ric_object_trans_set(&light_test->rico, &VEC3(0.0f, 4.0f, 0.0f));

    pkid lighting_test_cube_id = ric_load_object(
        pack_sav, OBJ_LIGHT_TEST_CUBE, sizeof(struct ric_object),
        "Lighting test cube");
    struct ric_object *lighting_test_cube =
        ric_pack_lookup(lighting_test_cube_id);
    ric_object_trans_set(lighting_test_cube, &VEC3(0.0f, 2.0f, 0.0f));

    /*
    pkid panel_id = ric_load_object(pack_sav, OBJ_GAME_PANEL,
                                     sizeof(struct game_panel), "panel");
    struct game_panel *panel = ric_pack_lookup(panel_id);
    ric_object_mesh_set(&panel->rico, mesh_panel_id);

    pkid button_id;
    struct game_button *button;
    for (u32 i = 0; i < ARRAY_COUNT(panel->buttons); i++)
    {
        button_id = ric_load_object(pack_sav, OBJ_GAME_BUTTON,
                                     sizeof(struct game_button), "button");
        button = ric_pack_lookup(button_id);
        ric_object_mesh_set(&button->rico, mesh_button_id);
        button->panel_id = panel_id;
        button->index = i;
        panel->buttons[i] = button_id;
    }
    */

    ric_pack_save(pack_dat, false);
    ric_pack_free(pack_dat);
    ric_pack_save(pack_sav, false);
    ric_pack_free(pack_sav);
}