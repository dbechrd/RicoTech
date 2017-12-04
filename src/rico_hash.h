#ifndef RICO_HASH_H
#define RICO_HASH_H

struct hash_table {
    struct hnd hnd;
    u32 count;
    // TODO: Replace linear search/insert with internal chaining
    struct hash_kv *slots;
};

void hashtable_init(struct hash_table *table, const char *name, u32 count);
void hashtable_free(struct hash_table *table);
void *hashtable_search(struct hash_table *table, void *key, u32 len);
int hashtable_insert(struct hash_table *table, void *key, u32 len, void *val);
bool hashtable_delete(struct hash_table *table, void *key, u32 len);

void rico_hashtable_init();

#endif // RICO_HASH_H
