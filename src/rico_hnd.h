#ifndef UID_H
#define UID_H

typedef u32 uid;
struct hnd
{
    uid uid;
    enum rico_hnd_type type;
    struct rico_chunk *chunk;
    struct rico_pool *pool;
    struct pool_id id;
    u32 ref_count;
    char name[32];
    u32 len;
};
#define UID_NULL 0
#define UID_BASE 1000

void hnd_init(struct hnd *hnd, enum rico_hnd_type type, const char *name);
//SERIAL(hnd_serialize);
//DESERIAL(hnd_deserialize);

#endif