#ifndef PACK_BUILDER_H
#define PACK_BUILDER_H

enum DEFAULT_IDS
{
    FONT_DEFAULT          = 0x01000001,
    FONT_DEFAULT_MATERIAL = 0x01000004,
    TEXTURE_DEFAULT_DIFF  = 0x01000005,
    TEXTURE_DEFAULT_SPEC  = 0x01000006,
    MATERIAL_DEFAULT      = 0x01000007,
    MESH_DEFAULT_BBOX     = 0x01000008,
    MESH_DEFAULT_SPHERE   = 0x01000009
};

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
    u32 id;
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
    RICO_ASSERT(pack->index[index].size < 0x00FFFFFF);
    return pack->buffer + pack->index[index].offset;
}

internal inline void *pack_lookup(struct pack *pack, u32 id)
{
    u32 pack_id = id & pack->id;
    RICO_ASSERT(pack_id == pack->id);

    u32 blob_id = id ^ pack->id;
    RICO_ASSERT(blob_id < pack->blob_count);
    
    return pack_read(pack, pack->lookup[blob_id]);
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

    id = id | pack->id;
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
