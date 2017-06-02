#ifndef RICO_HASH_H
#define RICO_HASH_H

typedef u32 hash_key;

struct hash_keyval {
    hash_key key;
    u32 handle;
};

struct hash_table {
    struct rico_uid uid;
    u32 count;
    struct hash_keyval *slots;
};

inline hash_key hashgen_str(const char *str);
inline hash_key hashgen_strlen(const char *str, int len);

void hashtable_init(struct hash_table *table, const char *name, u32 count);
void hashtable_free(struct hash_table *table);
u32 hashtable_search(struct hash_table *table, hash_key key);
u32 hashtable_search_by_name(struct hash_table *table, const char *name);
int hashtable_insert(struct hash_table *table, hash_key key, u32 handle);
bool hashtable_delete(struct hash_table *table, hash_key key);

// TODO: Where should global hash tables actually live?
extern struct hash_table global_textures;
extern struct hash_table global_meshes;
void rico_hashtable_init();

#endif // RICO_HASH_H
