inline hash_key hashgen_str(const char *str)
{
    hash_key key;
    MurmurHash3_x86_32(str, strlen(str), &key);
    return key;
}

inline hash_key hashgen_strlen(const char *str, int len)
{
    hash_key key;
    MurmurHash3_x86_32(str, len, &key);
    return key;
}

void hashtable_init(struct hash_table *table, const char *name, u32 count)
{
    uid_init(&table->uid, RICO_UID_HASHTABLE, name, false);
    table->count = count;
    table->slots = calloc(count, sizeof(table->slots[0]));
}

void hashtable_free(struct hash_table *table)
{
    free(table->slots);
}

inline u32 hash_code(struct hash_table *table, hash_key key)
{
    return key % table->count;
}

// TODO: Replace linear search/insert with quadratic if necessary
u32 hashtable_search(struct hash_table *table, hash_key key)
{
    u32 start_index = hash_code(table, key);
    u32 index = start_index;

    // Initial check to keep loop clean
    if (table->slots[index].handle == 0)
        return 0;

    while (1)
    {
        // Compare key; return if match
        if (table->slots[index].key == key)
            return table->slots[index].handle;

        // Next slot
        index = ++index % table->count;

        // Empty slot or back at start; not found
        if (table->slots[index].handle == 0 || index == start_index)
            return 0;
    }
}

u32 hashtable_search_by_name(struct hash_table *table, const char *name)
{
    hash_key key = hashgen_str(name);
    u32 handle = hashtable_search(table, key);
    return handle;
}

// TODO: Replace linear search/insert with quadratic if necessary
int hashtable_insert(struct hash_table *table, hash_key key, u32 handle)
{
    u32 start_index = hash_code(table, key);
    u32 index = start_index;

    while (table->slots[index].handle != 0 && table->slots[index].key != key)
    {
        // Next slot
        index = ++index % table->count;

        // Back at start; hash table full
        if (index == start_index)
            return RICO_ERROR(ERR_HASH_TABLE_FULL,
                              "Failed to insert into full hash table %s",
                              table->uid.name);
    }

#if RICO_DEBUG_HASH
    if (table->slots[index].key == key)
    {
        printf("[hash][WARN] uid=%d name=%s key=%d Overwriting existing key\n",
               table->uid.uid, table->uid.name, key);
    }
#endif

    // Empty slot found; insert
    table->slots[index].key = key;
    table->slots[index].handle = handle;

    return SUCCESS;
}

bool hashtable_delete(struct hash_table *table, hash_key key)
{
    u32 start_index = hash_code(table, key);
    u32 index = start_index;

    // Initial check to keep loop clean
    if (table->slots[index].handle == 0)
        return false;

    while (1)
    {
        // Compare key; delete if match
        if (table->slots[index].key == key)
        {
            table->slots[index].key = 0;
            table->slots[index].handle = 0;
            return true;
        }

        // Next slot
        index = ++index % table->count;

        // Empty slot or back at start; not found
        if (table->slots[index].handle == 0 || index == start_index)
            return false;
    }
}

///|////////////////////////////////////////////////////////////////////////////

// TODO: Where should global hash tables actually live?
struct hash_table global_strings;
struct hash_table global_fonts;
struct hash_table global_textures;
struct hash_table global_materials;
struct hash_table global_meshes;
struct hash_table global_objects;

void rico_hashtable_init()
{
    //struct rico_chunk *chunk = chunk_active();
    //hashtable_init(&global_hash_textures, "Textures", chunk->count_textures);
    hashtable_init(&global_strings,   "Strings",   RICO_STRING_POOL_SIZE);
    hashtable_init(&global_fonts,     "Fonts",     RICO_FONT_POOL_SIZE);
    hashtable_init(&global_textures,  "Textures",  RICO_TEXTURE_POOL_SIZE);
    hashtable_init(&global_materials, "Materials", RICO_MATERIAL_POOL_SIZE);
    hashtable_init(&global_meshes,    "Meshes",    RICO_MESH_POOL_SIZE);
    hashtable_init(&global_objects,   "Objects",   RICO_OBJECT_POOL_SIZE);
}