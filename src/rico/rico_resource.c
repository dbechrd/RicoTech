static struct dlb_hash global_fonts;
static struct dlb_hash global_materials;
static struct dlb_hash global_meshes;
static struct dlb_hash global_textures;

static void rico_resource_init()
{
    hashtable_init(&global_fonts,     STRING(global_fonts),     256, 8);
    hashtable_init(&global_materials, STRING(global_materials), 256, 8);
    hashtable_init(&global_meshes,    STRING(global_meshes),    256, 8);
    hashtable_init(&global_textures,  STRING(global_textures),  256, 8);
}
