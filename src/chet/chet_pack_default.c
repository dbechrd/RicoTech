#include "chet_packs.h"
#include "SDL/SDL.h"

// TODO: This entire pack could be embedded as binary data in the .exe once
//       the contents are finalized. This would allow the engine to run even
//       when the data directory is missing.
void pack_build_default(struct pack_info **pack_table, const char *filename)
{
    PERF_START(pack_build);

    struct pack_info *entry = dlb_vec_alloc(*pack_table);
    entry->path = filename;
    entry->id = ric_pack_init(RIC_PACK_ID_DEFAULT, entry->path, 16, MB(1));
    DLB_ASSERT(entry->id == RIC_PACK_ID_DEFAULT); // RIC_PACK_ID_DEFAULT *must* be id 0!

    global_default_font = ric_load_font_file(entry->id, "[global_default_font]", "font/cousine_regular.bff");
    struct ric_font *font = ric_pack_lookup(global_default_font);
    global_default_font_texture = font->tex_id;

    //pkid diff_id = ric_load_texture_file(pack, "[TEX_DIFF_DEFAULT]", "texture/pbr_default_0.tga");
    //pkid spec_id = ric_load_texture_file(pack, "[TEX_SPEC_DEFAULT]", "texture/pbr_default_1.tga");
    //pkid emis_id = ric_load_texture_file(pack, "[TEX_EMIS_DEFAULT]", "texture/pbr_default_2.tga");
    global_default_texture_diff = ric_load_texture_color(entry->id, "[DEFAULT_TEX_DIFF]", &COLOR_WHITE);
    global_default_texture_spec = ric_load_texture_color(entry->id, "[DEFAULT_TEX_SPEC]", &COLOR_GREEN);
    global_default_texture_emis = ric_load_texture_color(entry->id, "[DEFAULT_TEX_EMIS]", &COLOR_TRANSPARENT);
    global_default_material = ric_load_material(entry->id, "[DEFAULT_MATERIAL]", global_default_texture_diff,
                                                global_default_texture_spec, global_default_texture_emis);

    // HACK: This is a bit of a gross way to get the id of the last mesh
    ric_load_obj_file(entry->id, "mesh/prim_cube.obj", &global_default_mesh_cube, PROG_PBR);
    ric_load_obj_file(entry->id, "mesh/prim_sphere.obj", &global_default_mesh_sphere, PROG_PBR);

    pkid tex_transparent_id = ric_load_texture_color(entry->id, "[TEX_TRANSPARENT]", &COLOR_TRANSPARENT);
    pkid tex_translucent_id = ric_load_texture_color(entry->id, "[TEX_TRANSPLUCENT]", &COLOR_TRANSLUCENT);
    pkid mat_transparent_id = ric_load_material(entry->id, "[MAT_TRANSPARENT]", tex_translucent_id,
                                                tex_transparent_id, tex_transparent_id);

    ric_pack_save(entry->id, true);
    ric_pack_free(entry->id);

    PERF_END_MSG(pack_build, "Built pack '%s'\n", filename);
}