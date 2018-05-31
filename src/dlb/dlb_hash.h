#ifndef DLB_HASH_H
#define DLB_HASH_H

struct dlb_hash_kv
{
    u32 key;
    void *value;
};

struct dlb_hash
{
    const char *name;
    u32 count;
    // TODO: Replace linear search/insert with internal chaining
    struct dlb_hash_kv *slots;
};

static void rico_hashtable_init();
static void hashtable_init(struct dlb_hash *table, const char *name,
                           u32 count);
static void hashtable_free(struct dlb_hash *table);
static void *hashtable_search(struct dlb_hash *table, u32 key);
static int hashtable_insert(struct dlb_hash *table, u32 key,
                            void *value);
static bool hashtable_delete(struct dlb_hash *table, u32 key);

static inline u32 hash_u32(u32 key)
{
    u32 hash;
    MurmurHash3_x86_32(&key, sizeof(key), &hash);
    return hash;
}

static inline u32 hash_string(u32 len, const void *str)
{
    u32 hash;
    MurmurHash3_x86_32(str, len, &hash);
    return hash;
}

void hashtable_init(struct dlb_hash *table, const char *name, u32 count)
{
    table->name = name;
    table->count = count;
    table->slots = calloc(count, sizeof(table->slots[0]));

#if RICO_DEBUG_HASH
    printf("[hash][init] %s\n", table->hnd.name);
#endif
}

void hashtable_free(struct dlb_hash *table)
{
#if RICO_DEBUG_HASH
    printf("[hash][free] %s\n", table->hnd.name);
#endif

    free(table->slots);
}

void *hashtable_search(struct dlb_hash *table, u32 key)
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
int hashtable_insert(struct dlb_hash *table, u32 key, void *value)
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
                              "Failed to insert because %s is full",
                              table->name);
        }
    }

    // Empty slot found; insert
    table->slots[index].key = key;
    table->slots[index].value = value;

    return SUCCESS;
}
static bool hashtable_delete(struct dlb_hash *table, u32 key)
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

///|////////////////////////////////////////////////////////////////////////////

#endif
