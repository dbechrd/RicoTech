static dlb_hash global_fonts;
static dlb_hash global_materials;
static dlb_hash global_meshes;
static dlb_hash global_textures;

static void rico_resource_init()
{
    dlb_hash_init(&global_fonts,     DLB_HASH_STRING, STRING(global_fonts),     256);
    dlb_hash_init(&global_materials, DLB_HASH_STRING, STRING(global_materials), 256);
    dlb_hash_init(&global_meshes,    DLB_HASH_STRING, STRING(global_meshes),    256);
    dlb_hash_init(&global_textures,  DLB_HASH_STRING, STRING(global_textures),  256);
}
