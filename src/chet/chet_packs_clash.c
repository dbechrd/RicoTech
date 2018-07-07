#include "chet_packs.h"

void pack_build_clash_of_cubes(struct pack_info *pack_info)
{
    u32 pack = RICO_pack_init(0, pack_info->path_pak, 64, MB(32));
    u32 pack_sav = RICO_pack_init(0, pack_info->path_sav, 64, MB(32));

    /*
    pkid cube_mesh_id;
    RICO_load_obj_file(pack, "mesh/alpha_game_button.obj", &cube_mesh_id,
                       PROG_PBR);
    DLB_ASSERT(cube_mesh_id);

    pkid cube_id;
    struct small_cube *cube;

    static pkid small_cubes[2];
    for (u32 i = 0; i < ARRAY_COUNT(small_cubes); i++)
    {
        cube_id = RICO_load_object(pack_sav, OBJ_SMALL_CUBE,
                                   sizeof(struct small_cube), "small_cube");
        cube = RICO_pack_lookup(cube_id);
        RICO_object_mesh_set(&cube->rico, cube_mesh_id);
        RICO_object_trans_set(&cube->rico, &VEC3(-2.0f, i * 1.0f, 0.0f));
        small_cubes[i] = cube_id;
    }
    */

    RICO_pack_save(pack, false);
    RICO_pack_free(pack);
    RICO_pack_save(pack_sav, false);
    RICO_pack_free(pack_sav);
}