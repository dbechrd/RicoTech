#ifndef DLB_HASH_H
#define DLB_HASH_H

// S = "slot" (struct dlb_hash_entry, a key/value bucket)
// S0 = First bucket, w/ linked list which points to tail
// S0[1] = Second element of slot 0's chain (first in external chain)
//
// | Hash table      | External chains (pre-allocted, contiguous memory)     |
// | S0 S1 S2 ... Sn | S0[1] -> S0[2] -> ... -> S0[n] | S1[1] -> S1[2]-> ... |
//   |  |              ^                                ^
//   |  |  S0->next    |                                |
//   \--+--------------/            S1->next            |
//      \-----------------------------------------------/

// Key/value slot (a.k.a. "bucket")
struct dlb_hash_entry
{
    u32 key;
    void *value;
};

struct dlb_hash
{
    const char *name;
    u32 bucket_count;
    u32 chain_length;
    struct dlb_hash_entry *buckets;
};

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

static inline struct dlb_hash_entry *chains(struct dlb_hash *table)
{
    return table->buckets + table->bucket_count;
}

static void hashtable_init(struct dlb_hash *table, const char *name,
                           u32 bucket_count, u32 chain_length)
{
    DLB_ASSERT(bucket_count);
    DLB_ASSERT(chain_length >= 2);

    table->name = name;
    table->bucket_count = bucket_count;
    table->chain_length = chain_length;
    table->buckets = calloc(bucket_count * (1 + chain_length),
                            sizeof(table->buckets[0]));

#if DLB_HASH_DEBUG
    printf("[hash][init] %s\n", table->hnd.name);
#endif
}

void hashtable_free(struct dlb_hash *table)
{
#if DLB_HASH_DEBUG
    printf("[hash][free] %s\n", table->hnd.name);
#endif

    free(table->buckets);
}

void *hashtable_search(struct dlb_hash *table, u32 key)
{
    DLB_ASSERT(key);
    u32 hash = hash_u32(key);
    u32 index = hash % table->bucket_count;

    // Check if bucket empty
    if (!table->buckets[index].key)
    {
        return NULL;
    }

    // Check bucket head
    if (table->buckets[index].key == key)
    {
        return table->buckets[index].value;
    }

    u32 i = 0;
    struct dlb_hash_entry *entry = chains(table);
    while (i++ < table->chain_length && entry->key && entry->key != key)
    {
        entry++;
    }

    if (i < table->chain_length && entry->key)
    {
        return entry->value;
    }
    return NULL;
}

bool hashtable_insert(struct dlb_hash *table, u32 key, void *value)
{
    DLB_ASSERT(key);
    u32 hash = hash_u32(key);
    u32 index = hash % table->bucket_count;

    // Prevent dupe keys
    DLB_ASSERT(table->buckets[index].key != key);

    // Check bucket head
    if (!table->buckets[index].key)
    {
        table->buckets[index].key = key;
        table->buckets[index].value = value;
        return true;
    }

    u32 i = 0;
    struct dlb_hash_entry *entry = chains(table);
    while (i++ < table->chain_length && entry->key)
    {
        DLB_ASSERT(entry->key != key);  // Prevent dupe keys
        entry++;
    }

    if (i < table->chain_length)
    {
        entry->key = key;
        entry->value = value;
        return true;
    }
    return false;
}
static bool hashtable_delete(struct dlb_hash *table, u32 key)
{
    DLB_ASSERT(key);
    u32 hash = hash_u32(key);
    u32 index = hash % table->bucket_count;

    // Check if bucket empty
    if (!table->buckets[index].key)
    {
        return false;
    }

    struct dlb_hash_entry *prev_entry = chains(table);
    struct dlb_hash_entry *entry = prev_entry + 1;
    bool found = false;

    // Check bucket head
    if (table->buckets[index].key == key)
    {
        table->buckets[index].key = prev_entry->key;
        table->buckets[index].value = prev_entry->value;
        found = true;
    }

    // Keep chain tightly packed
    u32 i = 0;
    while (i++ < table->chain_length)
    {
        if (found)
        {
            prev_entry->key = entry->key;
            prev_entry->value = entry->value;
        }
        else
        {
            found = entry->key == key;
        }

        // The rest of the list is empty, skip it
        if (!entry->key) break;

        prev_entry++;
        entry++;
    }

    return found;
}

#endif