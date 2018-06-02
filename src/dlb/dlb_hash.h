#ifndef DLB_HASH_H
#define DLB_HASH_H

// S = "slot" (struct dlb_hash_kv, a key/value bucket)
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
struct dlb_hash_kv
{
    u32 key;
    void *value;
    struct dlb_hash_kv *next;
};

struct dlb_hash
{
    const char *name;
    u32 count;
    // TODO: Replace linear search/insert with internal chaining
    struct dlb_hash_kv *slots;
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

static void hashtable_init(struct dlb_hash *table, const char *name,
                           u32 count, u32 chain_length)
{
    table->name = name;
    table->count = count;
    table->slots = calloc(count * chain_length, sizeof(table->slots[0]));

    // Chains are stored at end of hash table memory:
    // Hash table        | Chains (kv[0][1] == kv[0].next)
    // kv[0] kv[1] kv[2] | kv[0][1] kv[0][2] ... kv[1][1] kv[1][2] ...
    struct dlb_hash_kv *chains = table->slots + count;
    struct dlb_hash_kv *kv;
    for (u32 slot = 0; slot < count; ++slot)
    {
        // Find start of this slot's chain tail
        kv = chains + (slot * (chain_length - 1));
        for (u32 i = 0; i < chain_length - 2; ++i)
        {
            // DEBUG: Delete me
            //kv->key = slot;
            //kv->value = (void *)(i + 1);

            kv->next = kv + 1;
            kv = kv->next;
        }

        // DEBUG: Delete me
        //kv->key = slot;
        //kv->value = (void *)(chain_length - 1);

        kv->next = 0;
    }

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
    u32 index = hash % table->count;

    struct dlb_hash_kv *kv = &table->slots[index];
    while (kv && kv->key != key)
    {
        kv = kv->next;
    }

    return kv ? kv->value : NULL;
}

bool hashtable_insert(struct dlb_hash *table, u32 key, void *value)
{
    u32 hash = hash_u32(key);
    u32 index = hash % table->count;

    struct dlb_hash_kv *kv = &table->slots[index];
    while (kv && kv->key != key)
    {
        kv = kv->next;
    }

    // Enforce unique key constraint
    RICO_ASSERT(!(kv && kv->key == key));

    if (kv)
    {
        kv->key = key;
        kv->value = value;
    }

    return kv != NULL;
}
static bool hashtable_delete(struct dlb_hash *table, u32 key)
{
    u32 hash = hash_u32(key);
    u32 index = hash % table->count;

    struct dlb_hash_kv *kv = &table->slots[index];
    while (kv && kv->key != key)
    {
        kv = kv->next;
    }

    if (kv)
    {
        kv->key = 0;
        kv->value = 0;
    }
    bool found = kv != NULL;

    // Keep kv tightly packed
    while (kv && kv->next)
    {
        kv->key = kv->next->key;
        kv->value = kv->next->value;
    }
    if (kv)
    {
        kv->key = 0;
        kv->value = 0;
    }

    return found;
}

#endif