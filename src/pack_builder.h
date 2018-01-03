#ifndef PACK_BUILDER_H
#define PACK_BUILDER_H

#define ID_BLOB_BITS 24
#define ID_BLOB_MASK ((1 << ID_BLOB_BITS) - 1)
#define ID_PACK_MASK (0xffffffff ^ ID_BLOB_MASK)
#define ID_PACK(id) ((id & ID_PACK_MASK) >> ID_BLOB_BITS)
#define ID_BLOB(id) (id & ID_BLOB_MASK)
#define ID_GENERATE(pack, blob) ((pack << ID_BLOB_BITS) | blob)

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
global void *pack_next(struct pack *pack, u32 id, enum rico_hnd_type type);
global void *pack_prev(struct pack *pack, u32 id, enum rico_hnd_type type);
global int pack_load(const char *filename, struct pack **_pack);
internal void pack_expand(struct pack *pack);
internal void pack_compact_buffer(struct pack *pack);
global void pack_delete(struct pack *pack, u32 id, enum rico_hnd_type type);

global u32 load_object(struct pack *pack, const char *name,
                       enum rico_obj_type type, u32 prop_count,
                       struct obj_property *props, const struct bbox *bbox);
global u32 load_texture(struct pack *pack, const char *name, GLenum target,
                        u32 width, u32 height, u8 bpp, u8 *pixels);
global u32 load_texture_file(struct pack *pack, const char *name,
                             const char *filename);
global u32 load_texture_color(struct pack *pack, const char *name,
                              struct vec4 color);
global u32 load_material(struct pack *pack, const char *name, u32 tex0,
                         u32 tex1);
global u32 load_font(struct pack *pack, const char *name,
                     const char *filename, u32 *font_mat);
global u32 load_mesh(struct pack *pack, const char *name, u32 vertex_count,
                     const struct rico_vertex *vertex_data, u32 element_count,
                     const GLuint *element_data);

internal inline void *pack_push(struct pack *pack, u32 bytes)
{
    RICO_ASSERT(pack->buffer_used + bytes < pack->buffer_size);
    void *ptr = pack->buffer + pack->buffer_used;
    pack->buffer_used += bytes;
    pack->index[pack->lookup[pack->blob_current_id]].size += bytes;
    return ptr;
}
internal inline void *pack_push_data(struct pack *pack, const void *data,
                                     u32 count, u32 size)
{
    u32 bytes = count * size;
    void *ptr = pack_push(pack, bytes);
    memcpy(ptr, data, bytes);
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
#define push_data(pack, data, count, size) (pack_push_data(pack, data, count, size))
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
    return pack->buffer + pack->index[index].offset;
}

global inline void *pack_lookup(struct pack *pack, u32 id)
{
    u32 pack_id = ID_PACK(id);
    RICO_ASSERT(pack_id == pack->id);

    u32 blob_id = ID_BLOB(id);
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

    id = ID_GENERATE(pack->id, id);
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