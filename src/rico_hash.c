typedef u32 hash;

// "dog" -> struct *texture
struct hash_kv {
    u32 len; // 4
    void *key;   // ['d', 'o', 'g', '\0']
    void *val; // 0x8b1453a3
};

internal inline bool keys_equal(struct hash_kv *kv, void *key, u32 len)
{
    return kv->len == len && memcmp(kv->key, key, len) == 0;
}

void hashtable_init(struct hash_table *table, const char *name, u32 count)
{
    hnd_init(&table->hnd, RICO_HND_HASHTABLE, name);
    table->count = count;
    table->slots = calloc(count, sizeof(table->slots[0]));
}

void hashtable_free(struct hash_table *table)
{
    free(table->slots);
}

void *hashtable_search(struct hash_table *table, void *key, u32 len)
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

// TODO: Replace linear search/insert with quadratic if necessary
int hashtable_insert(struct hash_table *table, void *key, u32 len, void *val)
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
    if (keys_equal(&table->slots[index].key, key, len))
    {
        printf("[hash][WARN] uid=%d name=%s key=%d Overwriting existing key\n",
               table->hnd.uid, table->hnd.name, key);
    }
#endif

    // Empty slot found; insert
    table->slots[index].len = len;
    table->slots[index].key = key;
    table->slots[index].val = val;

    return SUCCESS;
}

bool hashtable_delete(struct hash_table *table, void *key, u32 len)
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
    hashtable_init(&global_strings,   "Strings",   2 * RICO_POOL_SIZE_STRING);
    hashtable_init(&global_fonts,     "Fonts",     2 * RICO_POOL_SIZE_FONT);
    hashtable_init(&global_textures,  "Textures",  2 * RICO_POOL_SIZE_TEXTURE);
    hashtable_init(&global_materials, "Materials", 2 * RICO_POOL_SIZE_MATERIAL);
    hashtable_init(&global_meshes,    "Meshes",    2 * RICO_POOL_SIZE_MESH);
    hashtable_init(&global_objects,   "Objects",   2 * RICO_POOL_SIZE_OBJECT);
}
