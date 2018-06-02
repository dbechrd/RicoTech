static struct dlb_hash global_fonts;
static struct dlb_hash global_materials;
static struct dlb_hash global_meshes;
static struct dlb_hash global_objects;
static struct dlb_hash global_strings;
static struct dlb_hash global_string_slots;
static struct dlb_hash global_textures;

static void rico_hashtable_init()
{
    hashtable_init(&global_fonts,        STRING(global_fonts),        256, 8);
    hashtable_init(&global_materials,    STRING(global_materials),    256, 8);
    hashtable_init(&global_meshes,       STRING(global_meshes),       256, 8);
    hashtable_init(&global_objects,      STRING(global_objects),      256, 8);
    hashtable_init(&global_strings,      STRING(global_strings),      256, 8);
    hashtable_init(&global_string_slots, STRING(global_string_slots),  16, 8);
    hashtable_init(&global_textures,     STRING(global_textures),     256, 8);
}
