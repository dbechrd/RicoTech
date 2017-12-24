#ifndef PACK_BUILDER_H
#define PACK_BUILDER_H

const u32 FONT_DEFAULT = 1;
const u32 FONT_DEFAULT_TEX_DIFF = 2;
const u32 TEXTURE_DEFAULT_DIFF = 3;
const u32 TEXTURE_DEFAULT_SPEC = 4;
const u32 MATERIAL_DEFAULT = 5;
const u32 FONT_DEFAULT_MATERIAL = 6;
const u32 MESH_DEFAULT_BBOX = 7;
const u32 MESH_DEFAULT_SPHERE = 8;

struct blob_index
{
    enum rico_hnd_type type;
    u32 offset;
    u32 size;
    u8 deleted;
};

// Memory layout
// -----------------------
// struct pack;
// struct pack_entry index[index_count];
// u8 data[data_size];
struct pack
{
    u8 magic[4];
    u32 version;
    u8 name[32];

    u32 blob_count;
    u32 blobs_used;
    u32 buffer_size;
    u32 buffer_used;

    u32 index_offset;
    u32 data_offset;

    struct blob_index *index;
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
inline void *pack_push_data(struct pack *pack, const void *data, u32 size)
{
    void *ptr = pack_push(pack, size);
    memcpy(ptr, data, size);
    return ptr;
}
inline void *pack_push_str(struct pack *pack, const char *str)
{
    u32 size = strlen(str) + 1;
    void *ptr = pack_push(pack, size);
    memcpy(ptr, str, size);
    return ptr;
}
#define push_struct(pack, type) ((type *)pack_push(pack, sizeof(type)))
#define push_bytes(pack, bytes) (pack_push(pack, bytes))
#define push_string(pack, str) (pack_push_str(pack, str))
#define push_data(pack, data, size) (pack_push_data(pack, data, size))
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
    RICO_ASSERT(!pack->index[id].deleted);
    return pack->buffer + pack->index[id].offset;
}

inline void pack_delete(struct pack *pack, u32 id)
{
    // TODO: Combine adjacent free slots? Or, garbage collect when buffer is
    //       full. If garbage collect too often, auto-expand buffer. If auto-
    //       expand > MAX_SIZE, throw error instead.
    if (!id) return;
    RICO_ASSERT(id < pack->blob_count);
    pack->index[id].type = RICO_HND_NULL;
    pack->index[id].deleted = true;
}

void pack_build_all();
int pack_load(const char *filename, struct pack **_pack);

#endif
