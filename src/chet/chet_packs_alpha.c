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
    u32 pack_dat = RICO_pack_init(0, pack_info->path_pak, 64, MB(128));

    pkid tex_bricks_diff = RICO_load_texture_file(
        pack_dat, "bricks_diff", "texture/cobble_diff.tga");
    pkid tex_bricks_mrao = RICO_load_texture_file(
        pack_dat, "bricks_mrao", "texture/cobble_mrao.tga");
    pkid tex_bricks_emis = RICO_load_texture_color(
        pack_dat, "bricks_emis", &COLOR_TRANSPARENT);
    pkid mat_bricks = RICO_load_material(
        pack_dat, "bricks", tex_bricks_diff, tex_bricks_mrao, tex_bricks_emis);
    pkid mat_timmy = RICO_load_material(pack_dat, "timmy", 0, 0, 0);

    DLB_ASSERT(tex_bricks_mrao);
    DLB_ASSERT(tex_bricks_diff);
    DLB_ASSERT(tex_bricks_emis);
    DLB_ASSERT(mat_bricks);
    DLB_ASSERT(mat_timmy);

    pkid tex_sky_posx = RICO_load_texture_file(
        pack_dat, "sky_posx", "texture/sky_posx.tga");
    pkid tex_sky_negx = RICO_load_texture_file(
        pack_dat, "sky_negx", "texture/sky_negx.tga");
    pkid tex_sky_posy = RICO_load_texture_file(
        pack_dat, "sky_posy", "texture/sky_posy.tga");
    pkid tex_sky_negy = RICO_load_texture_file(
        pack_dat, "sky_negy", "texture/sky_negy.tga");
    pkid tex_sky_posz = RICO_load_texture_file(
        pack_dat, "sky_posz", "texture/sky_posz.tga");
    pkid tex_sky_negz = RICO_load_texture_file(
        pack_dat, "sky_negz", "texture/sky_negz.tga");

    pkid mesh_terrain_id;
    pkid mesh_door_id;
    pkid mesh_panel_id;
    pkid mesh_button_id;
    RICO_load_obj_file(
        pack_dat, "mesh/alpha_terrain_001.obj", &mesh_terrain_id, PROG_PBR);
    RICO_load_obj_file(
        pack_dat, "mesh/alpha_door_001.obj", &mesh_door_id, PROG_PBR);
    RICO_load_obj_file(
        pack_dat, "mesh/alpha_game_panel.obj", &mesh_panel_id, PROG_PBR);
    RICO_load_obj_file(
        pack_dat, "mesh/alpha_game_button.obj", &mesh_button_id, PROG_PBR);
    RICO_load_obj_file(
        pack_dat, "mesh/alpha_staircase_001.obj", 0, PROG_PBR);
    RICO_load_obj_file(
        pack_dat, "mesh/alpha_wall_001.obj", 0, PROG_PBR);

    DLB_ASSERT(mesh_door_id);
    DLB_ASSERT(mesh_terrain_id);
    DLB_ASSERT(mesh_panel_id);
    DLB_ASSERT(mesh_button_id);

    u32 pack_sav = RICO_pack_init(0, pack_info->path_sav, 64, MB(32));
    pkid terrain_id = RICO_load_object(pack_sav, OBJ_TERRAIN, 0, "terrain");
    struct ric_object *terrain = RICO_pack_lookup(terrain_id);
    RICO_object_mesh_set(terrain, mesh_terrain_id);
    RICO_object_material_set(terrain, mat_bricks);
    terrain->select_ignore = true;

    pkid timmy_id = RICO_load_object(
        pack_sav, OBJ_TIMMY, sizeof(struct timmy), "timmy");
    struct timmy *timmy = RICO_pack_lookup(timmy_id);
    RICO_object_trans_set(&timmy->rico, &VEC3(-4.0f, 0.0f, 0.0f));
    RICO_object_mesh_set(&timmy->rico, mesh_door_id);
    RICO_object_material_set(&timmy->rico, mat_timmy);
    timmy->lights_on = true;
    timmy->audio_on = true;

    pkid light_test_id = RICO_load_object(
        pack_sav, OBJ_LIGHT_TEST, sizeof(struct light_test), "Test light");
    struct light_test *light_test = RICO_pack_lookup(light_test_id);
    RICO_object_trans_set(&light_test->rico, &VEC3(0.0f, 4.0f, 0.0f));

    pkid lighting_test_cube_id = RICO_load_object(
        pack_sav, OBJ_LIGHT_TEST_CUBE, sizeof(struct ric_object),
        "Lighting test cube");
    struct ric_object *lighting_test_cube =
        RICO_pack_lookup(lighting_test_cube_id);
    RICO_object_trans_set(lighting_test_cube, &VEC3(0.0f, 2.0f, 0.0f));

    /*
    pkid panel_id = RICO_load_object(pack_sav, OBJ_GAME_PANEL,
                                     sizeof(struct game_panel), "panel");
    struct game_panel *panel = RICO_pack_lookup(panel_id);
    RICO_object_mesh_set(&panel->rico, mesh_panel_id);

    pkid button_id;
    struct game_button *button;
    for (u32 i = 0; i < ARRAY_COUNT(panel->buttons); i++)
    {
        button_id = RICO_load_object(pack_sav, OBJ_GAME_BUTTON,
                                     sizeof(struct game_button), "button");
        button = RICO_pack_lookup(button_id);
        RICO_object_mesh_set(&button->rico, mesh_button_id);
        button->panel_id = panel_id;
        button->index = i;
        panel->buttons[i] = button_id;
    }
    */

    RICO_pack_save(pack_dat, false);
    RICO_pack_free(pack_dat);
    RICO_pack_save(pack_sav, false);
    RICO_pack_free(pack_sav);
}