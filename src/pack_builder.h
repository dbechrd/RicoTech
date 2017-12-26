#ifndef PACK_BUILDER_H
#define PACK_BUILDER_H

#define FONT_DEFAULT 1
#define FONT_DEFAULT_TEX_DIFF 2
#define TEXTURE_DEFAULT_DIFF 3
#define TEXTURE_DEFAULT_SPEC 4
#define MATERIAL_DEFAULT 5
#define FONT_DEFAULT_MATERIAL 6
#define MESH_DEFAULT_BBOX 7
#define MESH_DEFAULT_SPHERE 8

struct blob_index
{
    enum rico_hnd_type type;
    u32 offset;
    u32 size;
};

// Memory layout
// -----------------------
// struct pack;
// struct pack_entry index[index_count];
// struct pack_entry fast_index[index_count];
// u8 data[data_size];
struct pack
{
    char magic[4];
    u32 version;
    char name[32];

    u32 blob_current_id;
    u32 blob_count;
    u32 blobs_used;

    u32 buffer_size;
    u32 buffer_used;

    u32 lookup_offset;
    u32 index_offset;
    u32 data_offset;

    u32 *lookup;
    struct blob_index *index;
    u8 *buffer;
};

extern struct pack *pack_default;
extern struct pack *pack_active;
extern struct pack *pack_transient;
extern struct pack *pack_frame;

global void pack_build_all();
global int pack_load(const char *filename, struct pack **_pack);
global void pack_expand(struct pack *pack);
global void pack_compact_buffer(struct pack *pack);

internal inline void *pack_push(struct pack *pack, u32 size)
{
    RICO_ASSERT(pack->buffer_used + size < pack->buffer_size);
    void *ptr = pack->buffer + pack->buffer_used;
    pack->buffer_used += size;
    pack->index[pack->lookup[pack->blob_current_id]].size += size;
    return ptr;
}
internal inline void *pack_push_data(struct pack *pack, const void *data,
                                     u32 size)
{
    void *ptr = pack_push(pack, size);
    memcpy(ptr, data, size);
    return ptr;
}
internal inline void *pack_push_str(struct pack *pack, const char *str)
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

internal inline void *pack_pop(struct pack *pack, u32 id)
{
    // This pop nonsese just seems like a back idea.. probably not resetting
    // index_iter properly, who knows what other side effects.
    RICO_ASSERT(0);
    UNUSED(pack);
    UNUSED(id);
    return NULL;

#if 0
    u32 offset = pack->index[pack->lookup[id]].offset;
    RICO_ASSERT(offset <= pack->buffer_used);

    if (offset < pack->buffer_used)
    {
        memset(pack->buffer + offset, 0, pack->buffer_used - offset);
        pack->buffer_used = offset;
    }
    void *ptr = pack->buffer + pack->buffer_used;
    return ptr;
#endif
}

internal inline void *pack_read(struct pack *pack, u32 index)
{
    RICO_ASSERT(index > 0);
    RICO_ASSERT(index < pack->blobs_used);
    RICO_ASSERT(pack->index[index].type);
    RICO_ASSERT(pack->index[index].size < 500000);
    return pack->buffer + pack->index[index].offset;
}

internal inline void *pack_lookup(struct pack *pack, u32 id)
{
    RICO_ASSERT(id);
    RICO_ASSERT(id < pack->blob_count);
    return pack_read(pack, pack->lookup[id]);
}

internal inline void pack_delete(struct pack *pack, u32 id)
{
    if (id == 0) return;
    RICO_ASSERT(id < pack->blob_count);

    u32 index = pack->lookup[id];
    RICO_ASSERT(index > 0);
    RICO_ASSERT(index < pack->blobs_used);
    RICO_ASSERT(pack->index[index].type);

    // 7 used, delete 4
    // 0 1 2 3 4 5 6 7 8 9
    // a b c d - e f [blob = 4]
    // a b c d e e f [blob = 5]
    // a b c d e f f

    // Shift everything after the deleted blob index left
    for (u32 idx = index; idx < pack->blobs_used - 1; ++idx)
    {
        pack->index[idx] = pack->index[idx + 1];
    }
    pack->lookup[id] = 0;
    pack->blobs_used--;
    pack->index[pack->blobs_used].type = 0;

    // Update lookup table
    for (u32 i = 1; i < pack->blob_count; ++i)
    {
        if (pack->lookup[i] > index)
            pack->lookup[i]--;
    }
}

internal inline u32 blob_start(struct pack *pack, enum rico_hnd_type type)
{
    RICO_ASSERT(pack->blob_current_id == 0);
    RICO_ASSERT(pack->blobs_used < pack->blob_count);

    // Find first available lookup id
    u32 id = 1;
    while (pack->lookup[id])
    {
        RICO_ASSERT(id < pack->blob_count);
        id++;
    }

    pack->blob_current_id = id;
    pack->lookup[pack->blob_current_id] = pack->blobs_used;
    pack->index[pack->blobs_used].type = type;
    pack->index[pack->blobs_used].offset = pack->buffer_used;
    pack->index[pack->blobs_used].size = 0;

    return id;
}
internal inline u32 blob_offset(struct pack *pack)
{
    RICO_ASSERT(pack->blob_current_id);
    RICO_ASSERT(pack->index[pack->lookup[pack->blob_current_id]].offset <
                pack->buffer_used);
    u32 offset = pack->buffer_used -
                 pack->index[pack->lookup[pack->blob_current_id]].offset;
    return offset;
}
internal inline void blob_end(struct pack *pack)
{

    RICO_ASSERT(pack->blob_current_id);
    RICO_ASSERT(pack->lookup[pack->blob_current_id] == pack->blobs_used);
    u32 size = blob_offset(pack);

#if RICO_DEBUG_PACK
    static u32 max_alloc = 0;
    if (size > max_alloc)
    {
        max_alloc = size;
        printf("[PERF][Pack] New max alloc for '%s': %u bytes\n", pack->name,
               max_alloc);
    }
#endif

    pack->index[pack->blobs_used].size = size;
    pack->blobs_used++;
    pack->blob_current_id = 0;

    // Compact buffer if over 66% utilized
    u32 four_to_one = pack->buffer_used >> 1;
    if (four_to_one > pack->buffer_size - pack->buffer_used)
    {
        pack_compact_buffer(pack);
    }
}
internal inline void blob_error(struct pack *pack, u32 *pack_idx)
{
    pack_pop(pack, *pack_idx);
    *pack_idx = UINT_MAX;
}

#endif
