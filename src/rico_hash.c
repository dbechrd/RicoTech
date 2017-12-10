typedef u32 hash;

// "dog" -> struct *texture
struct hash_kv {
    u32 len;         // 4
    const void *key; // ['d', 'o', 'g', '\0']
    void *val;       // 0x8b1453a3
};

internal inline bool keys_equal(struct hash_kv *kv, const void *key, u32 len)
{
    return kv->len == len && memcmp(kv->key, key, len) == 0;
}

void hashtable_init(struct hash_table *table, const char *name, u32 count)
{
    hnd_init(&table->hnd, RICO_HND_HASHTABLE, name);
    table->count = count;
    table->slots = calloc(count, sizeof(table->slots[0]));

#if RICO_DEBUG_HASH
    printf("[hash][init] uid=%d name=%s\n", table->hnd.uid, table->hnd.name);
#endif
}

void hashtable_free(struct hash_table *table)
{
#if RICO_DEBUG_HASH
    printf("[hash][free] uid=%d name=%s\n", table->hnd.uid, table->hnd.name);
#endif

    free(table->slots);
}

internal void *hashtable_search(struct hash_table *table, const void *key,
                                u32 len)
{
    hash hash;
    MurmurHash3_x86_32(key, len, &hash);

    u32 start = hash % table->count;
    u32 index = start;

    do
    {
        if (!table->slots[index].val)
            break;

        // Compare keys; return if match
        if (keys_equal(&table->slots[index], key, len))
            return table->slots[index].val;

        // Next slot
		index++;
        index %= table->count;
    } while (index != start);

    // Back at start; not found
    return 0;
}

void *hashtable_search_str(struct hash_table *table, const char *str)
{
    void *val = hashtable_search(table, str, strlen(str));
#if RICO_DEBUG_HASH
    printf("[hash][srch] uid=%d name=%s [%s, %p]\n", table->hnd.uid,
            table->hnd.name, str, val);
#endif
    return val;
}

void *hashtable_search_hnd(struct hash_table *table, struct hnd *hnd)
{
    void *val = hashtable_search(table, hnd->name, hnd->len);
#if RICO_DEBUG_HASH
    printf("[hash][srch] uid=%d name=%s [%s, %p]\n", table->hnd.uid,
           table->hnd.name, hnd->name, val);
#endif
    return val;
}

// TODO: Replace linear search/insert with quadratic if necessary, or use
//       external chaining.
int hashtable_insert(struct hash_table *table, const void *key, u32 len,
                     void *val)
{
    hash hash;
    MurmurHash3_x86_32(key, len, &hash);

    u32 start = hash % table->count;
    u32 index = start;

    while (table->slots[index].val &&
           !keys_equal(&table->slots[index], key, len))
    {
        // Next slot
        index++;
		index %= table->count;

        // Back at start; hash table full
        if (index == start)
        {
            return RICO_ERROR(ERR_HASH_TABLE_FULL,
                              "Failed to insert into full hash table %s",
                              table->hnd.name);
        }
    }

#if RICO_DEBUG_HASH
    if (keys_equal(&table->slots[index], key, len))
    {
        return RICO_ERROR(ERR_HASH_OVERWRITE, "Overwriting existing key\n");
        //printf("[hash][WARN] uid=%d name=%s [%.*s] Overwriting existing key\n",
        //       table->hnd.uid, table->hnd.name, len, (const char *)key);
    }
#endif

    // Empty slot found; insert
    table->slots[index].len = len;
    table->slots[index].key = key;
    table->slots[index].val = val;

    return SUCCESS;
}

int hashtable_insert_str(struct hash_table *table, const char *str, void *val)
{
#if RICO_DEBUG_HASH
    printf("[hash][ins ] uid=%d name=%s [%s, %p]\n", table->hnd.uid,
           table->hnd.name, str, val);
#endif
    return hashtable_insert(table, str, strlen(str), val);
}

int hashtable_insert_hnd(struct hash_table *table, struct hnd *hnd, void *val)
{
#if RICO_DEBUG_HASH
    printf("[hash][ins ] uid=%d name=%s [%s, %p]\n", table->hnd.uid,
           table->hnd.name, hnd->name, val);
#endif
    return hashtable_insert(table, hnd->name, hnd->len, val);
}

bool hashtable_delete(struct hash_table *table, const void *key, u32 len)
{
    hash hash;
    MurmurHash3_x86_32(key, len, &hash);

    u32 start = hash % table->count;
    u32 index = start;

    do
    {
        if (!table->slots[index].val)
            break;

        // Compare keys; return if match
        if (keys_equal(&table->slots[index], key, len))
        {
            table->slots[index].len = 0;
            table->slots[index].key = 0;
            table->slots[index].val = 0;
            return true;
        }

        // Next slot
		index++;
        index %= table->count;
    } while (index != start);

    // Back at start; not found
    return false;
}

bool hashtable_delete_str(struct hash_table *table, const char *str)
{

    bool success = hashtable_delete(table, str, strlen(str));
#if RICO_DEBUG_HASH
    printf("[hash][del ] uid=%d name=%s [%s, %d]\n", table->hnd.uid,
           table->hnd.name, str, success);
#endif
    return success;
}

bool hashtable_delete_hnd(struct hash_table *table, struct hnd *hnd)
{
    bool success = hashtable_delete(table, hnd->name, hnd->len);
#if RICO_DEBUG_HASH
    printf("[hash][del ] uid=%d name=%s [%s, %d]\n", table->hnd.uid,
           table->hnd.name, hnd->name, success);
#endif
    return success;
}

///|////////////////////////////////////////////////////////////////////////////

// TODO: Where should global hash tables actually live?
struct hash_table global_strings;
struct hash_table global_fonts;
struct hash_table global_textures;
struct hash_table global_materials;
struct hash_table global_meshes;
struct hash_table global_objects;
struct hash_table global_string_slots;

void rico_hashtable_init()
{
    hashtable_init(&global_strings,   "Strings",   256);
    hashtable_init(&global_fonts,     "Fonts",     256);
    hashtable_init(&global_textures,  "Textures",  256);
    hashtable_init(&global_materials, "Materials", 256);
    hashtable_init(&global_meshes,    "Meshes",    256);
    hashtable_init(&global_objects,   "Objects",   256);
    hashtable_init(&global_string_slots, "String Slots", 16);
}
