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
static void *hashtable_search(struct hash_table *table, const u32 key);
static int hashtable_insert(struct hash_table *table, const u32 key,
                            void *value);
static bool hashtable_delete(struct hash_table *table, const u32 key);

static inline const u32 hash_u32(const u32 key)
{
    u32 hash;
    MurmurHash3_x86_32(&key, sizeof(key), &hash);
    return hash;
}

static inline const u32 hash_string(const u32 len, const void *str)
{
    u32 hash;
    MurmurHash3_x86_32(str, len, &hash);
    return hash;
}

#if 0
static void *hashtable_search_str(struct hash_table *table, const char *str);
static int hashtable_insert_str(struct hash_table *table, const char *str,
                                const void *val, u32 len);
static bool hashtable_delete_str(struct hash_table *table, const char *str);
static void *hashtable_search_uid(struct hash_table *table,
                                  const struct uid *uid);
static int hashtable_insert_uid(struct hash_table *table, const struct uid *uid,
                         const void *val, u32 len);
static bool hashtable_delete_uid(struct hash_table *table,
                                 const struct uid *uid);
static void *hashtable_search_pkid(struct hash_table *table, pkid pkid);
static int hashtable_insert_pkid(struct hash_table *table, pkid pkid,
                                 const void *val, u32 vlen);
static bool hashtable_delete_pkid(struct hash_table *table, pkid pkid);
#endif

static void rico_hashtable_init();

#endif
