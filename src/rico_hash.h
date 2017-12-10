#ifndef RICO_HASH_H
#define RICO_HASH_H

struct hash_table {
    struct hnd hnd;
    u32 count;
    // TODO: Replace linear search/insert with internal chaining
    struct hash_kv *slots;
};

extern struct hash_table global_strings;
extern struct hash_table global_fonts;
extern struct hash_table global_textures;
extern struct hash_table global_materials;
extern struct hash_table global_meshes;
extern struct hash_table global_objects;

void hashtable_init(struct hash_table *table, const char *name, u32 count);
void hashtable_free(struct hash_table *table);
void *hashtable_search(struct hash_table *table, const void *key, u32 len);
void *hashtable_search_str(struct hash_table *table, const char *str);
void *hashtable_search_hnd(struct hash_table *table, struct hnd *hnd);
int hashtable_insert(struct hash_table *table, const void *key, u32 len,
                     void *val);
int hashtable_insert_str(struct hash_table *table, const char *str, void *val);
int hashtable_insert_hnd(struct hash_table *table, struct hnd *hnd, void *val);
bool hashtable_delete(struct hash_table *table, const void *key, u32 len);
bool hashtable_delete_str(struct hash_table *table, const char *str);
bool hashtable_delete_hnd(struct hash_table *table, struct hnd *hnd);

void rico_hashtable_init();

#endif // RICO_HASH_H
