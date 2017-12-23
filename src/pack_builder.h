#ifndef PACK_BUILDER_H
#define PACK_BUILDER_H

struct pack_entry
{
    enum rico_hnd_type type;
    u32 offset;
    u32 size;
};

// Memory layout
// -----------------------
// struct pack;
// struct pack_entry index[index_count];
// u8 data[data_size];
struct pack
{
    u32 magic;
    u32 version;
    u32 blob_count;
    u32 buffer_size;
    u32 buffer_used;

    u32 index_offset;
    u32 data_offset;

    struct pack_entry *index;
    u8 *buffer;
};

extern struct pack *pack_active;

inline void *pack_push(struct pack *pack, u32 size)
{
    RICO_ASSERT(pack->buffer_used + size < pack->buffer_size);
    void *ptr = pack->buffer + pack->buffer_used;
    pack->buffer_used += size;
    return ptr;
}
#define push_struct(pack, type) ((type *)pack_push(pack, sizeof(type)))
#define push_string(pack, str) (pack_push(pack, strlen(str) + 1))
#define push_bytes(pack, bytes) (pack_push(pack, bytes))
#define array_count(arr) (sizeof(arr) / sizeof(arr[0]))

inline u32 pack_offset(struct pack *pack, u32 start)
{
    RICO_ASSERT(start >= 0);
    RICO_ASSERT(start <= pack->buffer_used);
    return pack->buffer_used - start;
}

inline void *pack_pop(struct pack *pack, u32 offset)
{
    RICO_ASSERT(offset >= 0);
    RICO_ASSERT(offset <= pack->buffer_used);
    memset(pack->buffer + offset, 0, pack->buffer_used - offset);
    pack->buffer_used = offset;
    void *ptr = pack->buffer + pack->buffer_used;
    return ptr;
}

inline void *pack_read(struct pack *pack, u32 id)
{
    // TODO: If id == 0, return default resource of requested type
    RICO_ASSERT(id > 0);
    RICO_ASSERT(id < pack->blob_count);
    return pack->buffer + pack->index[id].offset;
}

int pack_build(const char *filename);

#endif
