static struct dlb_hash global_strings;
static struct dlb_hash global_fonts;
static struct dlb_hash global_textures;
static struct dlb_hash global_materials;
static struct dlb_hash global_meshes;
static struct dlb_hash global_objects;
//static struct dlb_hash global_uids;
static struct dlb_hash global_string_slots;

static void rico_hashtable_init()
{
    u32 strings   = 256;
    u32 fonts     = 256;
    u32 textures  = 256;
    u32 materials = 256;
    u32 meshes    = 256;
    u32 objects   = 256;
    u32 string_slots = 16;
    //int uids = strings + fonts + textures + materials + meshes + objects;

    hashtable_init(&global_strings,      "global_strings",      strings);
    hashtable_init(&global_fonts,        "global_fonts",        fonts);
    hashtable_init(&global_textures,     "global_textures",     textures);
    hashtable_init(&global_materials,    "global_materials",    materials);
    hashtable_init(&global_meshes,       "global_meshes",       meshes);
    hashtable_init(&global_objects,      "global_objects",      objects);
    hashtable_init(&global_string_slots, "global_string_slots", string_slots);
    //hashtable_init(&global_uids,      "global_uids",      uids);
}
