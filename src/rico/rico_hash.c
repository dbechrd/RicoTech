struct hash_kv
{
    u32 key;
    void *value;
};

//dlb_hash_table_insert(dlb_hash_str("Test"), &texture)
//dlb_hash_table_insert(dlb_hash_u32(texture.uid.pkid), &texture)

static void hashtable_init(struct hash_table *table, const char *name,
                           u32 count)
{
    table->name = name;
    table->count = count;
    table->slots = calloc(count, sizeof(table->slots[0]));

#if RICO_DEBUG_HASH
    printf("[hash][init] %s\n", table->hnd.name);
#endif
}
static void hashtable_free(struct hash_table *table)
{
#if RICO_DEBUG_HASH
    printf("[hash][free] %s\n", table->hnd.name);
#endif

    free(table->slots);
}
static void *hashtable_search(struct hash_table *table, u32 key)
{
    u32 hash = hash_u32(key);
    u32 start = hash % table->count;
    u32 index = start;

    do
    {
        if (table->slots[index].value == NULL)
            break;

        // Compare keys; return if match
        if (table->slots[index].key == key)
            return table->slots[index].value;

        // Next slot
		index++;
        index %= table->count;
    } while (index != start);

    // Back at start; not found
    return NULL;
}
// TODO: Replace linear search/insert with quadratic if necessary, or use
//       external chaining.
static int hashtable_insert(struct hash_table *table, u32 key, void *value)
{
    u32 hash = hash_u32(key);
    u32 start = hash % table->count;
    u32 index = start;

    while (table->slots[index].key != key &&
           table->slots[index].value != NULL)
    {
        // Next slot
        index++;
        index %= table->count;

        // Back at start; hash table full
        if (index == start)
        {
            return RICO_ERROR(ERR_HASH_TABLE_FULL,
                              "Failed to insert into full hash table %s",
                              table->name);
        }
    }

#if RICO_DEBUG_HASH
    if (table->slots[index].val)
    {
        //return RICO_ERROR(ERR_HASH_OVERWRITE, "Overwriting existing key\n");
        //printf("[hash][WARN] %s [%.*s] Overwriting existing key\n",
        //       table->hnd.uid, table->hnd.name, len, (const char *)key);
    }
#endif

    // Empty slot found; insert
    table->slots[index].key = key;
    table->slots[index].value = value;

    return SUCCESS;
}
static bool hashtable_delete(struct hash_table *table, u32 key)
{
    u32 hash = hash_u32(key);
    u32 start = hash % table->count;
    u32 index = start;

    do
    {
        if (table->slots[index].value == NULL)
            break;

        // Compare keys; return if match
        if (table->slots[index].key == key)
        {
            table->slots[index].key = 0;
            table->slots[index].value = 0;
            return true;
        }

        // Next slot
        index++;
        index %= table->count;
    } while (index != start);

    // Back at start; not found
    return false;
}
#if 0
static void *hashtable_search_str(struct hash_table *table, const char *str)
{
    void *val = hashtable_search(table, (u8 *)str, dlb_strlen(str));
#if RICO_DEBUG_HASH
    printf("[hash][srch] %s\n             [%s, %p]\n", table->hnd.name, str,
           val);
#endif
    return val;
}
static int hashtable_insert_str(struct hash_table *table, const char *str,
                                const void *val, u32 vlen)
{
#if RICO_DEBUG_HASH
    printf("[hash][ins ] %s\n             [%s, %p]\n", table->hnd.name, str,
           val);
#endif
    return hashtable_insert(table, (u8 *)str, dlb_strlen(str), val, vlen);
}
static bool hashtable_delete_str(struct hash_table *table, const char *str)
{

    bool success = hashtable_delete(table, (u8 *)str, dlb_strlen(str));
#if RICO_DEBUG_HASH
    printf("[hash][del ] %s\n             [%s, %d]\n", table->hnd.name, str,
           success);
#endif
    return success;
}
static void *hashtable_search_uid(struct hash_table *table,
                                  const struct uid *uid)
{
    void *val = hashtable_search(table, (u8 *)&uid, sizeof(uid));
#if RICO_DEBUG_HASH
    printf("[hash][srch] %s\n             [%x, %p]\n", table->hnd.name, uid,
           val);
#endif
    return val;
}
static int hashtable_insert_uid(struct hash_table *table, const struct uid *uid,
                                const void *val, u32 vlen)
{
#if RICO_DEBUG_HASH
    printf("[hash][ins ] %s\n             [%x, %p]\n", table->hnd.name, uid,
           val);
#endif
    return hashtable_insert(table, (u8 *)&uid, sizeof(uid), val, vlen);
}
static bool hashtable_delete_uid(struct hash_table *table,
                                 const struct uid *uid)
{
    bool success = hashtable_delete(table, (u8 *)&uid, sizeof(uid));
#if RICO_DEBUG_HASH
    printf("[hash][del ] %s\n             [%x, %d]\n", table->hnd.name, uid,
           success);
#endif
    return success;
}
static void *hashtable_search_pkid(struct hash_table *table, pkid pkid)
{
    void *val = hashtable_search(table, (u8 *)&pkid, sizeof(pkid));
#if RICO_DEBUG_HASH
    printf("[hash][srch] %s\n             [%d, %p]\n", table->hnd.name, pkid,
           val);
#endif
    return val;
}
static int hashtable_insert_pkid(struct hash_table *table, pkid pkid,
                                 const void *val, u32 vlen)
{
#if RICO_DEBUG_HASH
    printf("[hash][ins ] %s\n             [%d, %p]\n", table->hnd.name, pkid,
           val);
#endif
    return hashtable_insert(table, (u8 *)&pkid, sizeof(pkid), val, vlen);
}
static bool hashtable_delete_pkid(struct hash_table *table, pkid pkid)
{
    bool success = hashtable_delete(table, (u8 *)&pkid, sizeof(pkid));
#if RICO_DEBUG_HASH
    printf("[hash][del ] %s\n             [%d, %d]\n", table->hnd.name, pkid,
           success);
#endif
    return success;
}
#endif

///|////////////////////////////////////////////////////////////////////////////

// TODO: Where should global hash tables actually live?
static struct hash_table global_strings;
static struct hash_table global_fonts;
static struct hash_table global_textures;
static struct hash_table global_materials;
static struct hash_table global_meshes;
static struct hash_table global_objects;
//static struct hash_table global_uids;
static struct hash_table global_string_slots;

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
