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
extern struct hash_table global_uids;
extern struct hash_table global_string_slots;

void hashtable_init(struct hash_table *table, const char *name, u32 count);
void hashtable_free(struct hash_table *table);
void *hashtable_search_str(struct hash_table *table, const char *str);
void *hashtable_search_hnd(struct hash_table *table, struct hnd *hnd);
void *hashtable_search_uid(struct hash_table *table, uid uid);
int hashtable_insert_str(struct hash_table *table, const char *str, void *val);
int hashtable_insert_hnd(struct hash_table *table, struct hnd *hnd, void *val);
int hashtable_insert_uid(struct hash_table *table, uid uid, void *val);
bool hashtable_delete_str(struct hash_table *table, const char *str);
bool hashtable_delete_hnd(struct hash_table *table, struct hnd *hnd);
bool hashtable_delete_uid(struct hash_table *table, uid uid);

void rico_hashtable_init();

#endif // RICO_HASH_H
