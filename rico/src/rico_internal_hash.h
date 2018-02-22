#ifndef RICO_INTERNAL_HASH_H
#define RICO_INTERNAL_HASH_H

struct hash_table
{
    const char *name;
    u32 count;
    // TODO: Replace linear search/insert with internal chaining
    struct hash_kv *slots;
};

static struct hash_table global_strings;
static struct hash_table global_fonts;
static struct hash_table global_textures;
static struct hash_table global_materials;
static struct hash_table global_meshes;
static struct hash_table global_objects;
//static struct hash_table global_uids;
static struct hash_table global_string_slots;

static void hashtable_init(struct hash_table *table, const char *name,
                           u32 count);
static void hashtable_free(struct hash_table *table);
static void *hashtable_search(struct hash_table *table, const void *key,
                              u32 klen);
static int hashtable_insert(struct hash_table *table, const void *key, u32 klen,
                            const void *val, u32 vlen);
static bool hashtable_delete(struct hash_table *table, const void *key,
                             u32 klen);

static void *hashtable_search_str(struct hash_table *table, const char *str);
static int hashtable_insert_str(struct hash_table *table, const char *str,
                                const void *val, u32 len);
static bool hashtable_delete_str(struct hash_table *table, const char *str);
#if 0
static void *hashtable_search_uid(struct hash_table *table,
                                  const struct uid *uid);
static int hashtable_insert_uid(struct hash_table *table, const struct uid *uid,
                         const void *val, u32 len);
static bool hashtable_delete_uid(struct hash_table *table,
                                 const struct uid *uid);
#endif
static void *hashtable_search_pkid(struct hash_table *table, pkid pkid);
static int hashtable_insert_pkid(struct hash_table *table, pkid pkid,
                                 const void *val, u32 vlen);
static bool hashtable_delete_pkid(struct hash_table *table, pkid pkid);

static void rico_hashtable_init();

#endif