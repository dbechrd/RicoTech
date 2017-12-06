#ifndef UID_H
#define UID_H

#define RICO_HND_TYPES(f)    \
    f(RICO_HND_OBJECT)       \
    f(RICO_HND_TEXTURE)      \
    f(RICO_HND_MESH)         \
    f(RICO_HND_BBOX)         \
    f(RICO_HND_FONT)         \
    f(RICO_HND_STRING)       \
    f(RICO_HND_MATERIAL)     \
    f(RICO_HND_CEREAL_COUNT) \
    f(RICO_HND_CHUNK)        \
    f(RICO_HND_POOL)         \
    f(RICO_HND_HASHTABLE)    \
    f(RICO_HND_COUNT)

enum rico_hnd_type {
    RICO_HND_TYPES(GEN_LIST)
};
//extern const char *rico_hnd_type_string[];

typedef u32 uid;
struct hnd {
    enum rico_hnd_type type;
    uid uid;
    char name[32];
    u32 len;
};
#define UID_NULL 0
#define UID_BASE 1000

void hnd_init(struct hnd *_hnd, enum rico_hnd_type type, const char *name);
SERIAL(hnd_serialize);
DESERIAL(hnd_deserialize);

#endif // UID_H
