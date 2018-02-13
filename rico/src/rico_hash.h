#ifndef RICO_HASH_H
#define RICO_HASH_H

struct hash_table
{
    const char *name;
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
//extern struct hash_table global_uids;
extern struct hash_table global_string_slots;

void hashtable_init(struct hash_table *table, const char *name, u32 count);
void hashtable_free(struct hash_table *table);
void *hashtable_search(struct hash_table *table, const void *key, u32 klen);
int hashtable_insert(struct hash_table *table, const void *key, u32 klen,
                     const void *val, u32 vlen);
bool hashtable_delete(struct hash_table *table, const void *key, u32 klen);

void *hashtable_search_str(struct hash_table *table, const char *str);
int hashtable_insert_str(struct hash_table *table, const char *str,
                         const void *val, u32 len);
bool hashtable_delete_str(struct hash_table *table, const char *str);
#if 0
void *hashtable_search_uid(struct hash_table *table, const struct uid *uid);
int hashtable_insert_uid(struct hash_table *table, const struct uid *uid,
                         const void *val, u32 len);
bool hashtable_delete_uid(struct hash_table *table, const struct uid *uid);
#endif
void *hashtable_search_pkid(struct hash_table *table, pkid pkid);
int hashtable_insert_pkid(struct hash_table *table, pkid pkid, const void *val,
                          u32 vlen);
bool hashtable_delete_pkid(struct hash_table *table, pkid pkid);

void rico_hashtable_init();

#endif
