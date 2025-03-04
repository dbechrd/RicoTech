#include "chet_packs.h"

void pack_build_clash_of_cubes(struct pack_info **pack_table, const char *filename)
{
    /*
    PERF_START(pack_build);

    u32 pack = ric_pack_init(0, pack_info->path, 64, MB(32));

    pkid cube_mesh_id;
    ric_load_obj_file(pack, "mesh/alpha_game_button.obj", &cube_mesh_id,
                       PROG_PBR);
    DLB_ASSERT(cube_mesh_id);

    pkid cube_id;
    struct small_cube *cube;

    static pkid small_cubes[2];
    for (u32 i = 0; i < ARRAY_COUNT(small_cubes); i++)
    {
        cube_id = ric_load_object(pack, OBJ_SMALL_CUBE,
                                  sizeof(struct small_cube), "small_cube");
        cube = ric_pack_lookup(cube_id);
        ric_object_mesh_set(&cube->rico, cube_mesh_id);
        ric_object_trans_set(&cube->rico, &VEC3(-2.0f, i * 1.0f, 0.0f));
        small_cubes[i] = cube_id;
    }

    ric_pack_save(pack, false);
    ric_pack_free(pack);

    PERF_END_MSG(pack_build, "Built pack '%s'\n", filename);
    */
}