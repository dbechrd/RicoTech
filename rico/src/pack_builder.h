#ifndef PACK_BUILDER_H
#define PACK_BUILDER_H

#define PKID_BLOB_BITS 24
#define PKID_BLOB_MASK ((1 << PKID_BLOB_BITS) - 1)
#define PKID_PACK_MASK (0xffffffff ^ PKID_BLOB_MASK)
#define PKID_PACK(id) ((id & PKID_PACK_MASK) >> PKID_BLOB_BITS)
#define PKID_BLOB(id) (id & PKID_BLOB_MASK)
#define PKID_GENERATE(pack, blob) ((pack << PKID_BLOB_BITS) | blob)

enum PACK_IDS
{
    PACK_DEFAULT,
    PACK_TRANSIENT,
    PACK_FRAME,
    PACK_COUNT
};

enum DEFAULT_IDS
{
    FONT_DEFAULT = 1,
    FONT_DEFAULT_TEXTURE,
    TEXTURE_DEFAULT_DIFF,
    TEXTURE_DEFAULT_SPEC,
    TEXTURE_DEFAULT_EMIS,
    MATERIAL_DEFAULT,
    MESH_DEFAULT_CUBE,
    MESH_DEFAULT_SPHERE
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

#define MAX_PACKS 32
extern u32 packs_next;
extern struct pack *packs[MAX_PACKS];

internal void *blob_start(struct pack *pack, enum rico_hnd_type type,
                          const char *name);

internal void pack_expand(struct pack *pack);
internal void pack_compact_buffer(struct pack *pack);

struct pack *pack_init(u32 id, const char *name, u32 blob_count,
                       u32 buffer_size);
int pack_save(struct pack *pack, const char *filename, bool shrink);
int pack_load(const char *filename, struct pack **_pack);
void pack_free(u32 id);

global void *pack_next(pkid pkid, enum rico_hnd_type type);
global void *pack_prev(pkid pkid, enum rico_hnd_type type);
void pack_delete(pkid pkid);

pkid load_object(struct pack *pack, enum rico_obj_type type, const char *name);
pkid load_texture(struct pack *pack, const char *name, GLenum target,
                 u32 width, u32 height, u8 bpp, u8 *pixels);
pkid load_texture_file(struct pack *pack, const char *name,
                       const char *filename);
pkid load_texture_color(struct pack *pack, const char *name, struct vec4 color);
pkid load_material(struct pack *pack, const char *name, pkid tex0, pkid tex1,
                   pkid tex2);
pkid load_font(struct pack *pack, const char *name, const char *filename,
               pkid *font_tex);
pkid load_mesh(struct pack *pack, const char *name, u32 vertex_size,
               u32 vertex_count, const void *vertex_data,
               u32 element_count, const GLuint *element_data);
pkid load_string(struct pack *pack, const char *name,
                 enum rico_string_slot slot, float x, float y,
                 struct vec4 color, u32 lifespan, struct rico_font *font,
                 const char *text);
int load_obj_file(struct pack *pack, const char *filename, pkid *_last_mesh_id);
global void pack_build_default();

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
    u32 size = (u32)strlen(str) + 1;
    void *ptr = pack_push(pack, size);
    memcpy(ptr, str, size);
    return ptr;
}
#define push_struct(pack, type) ((type *)pack_push(pack, sizeof(type)))
#define push_bytes(pack, bytes) (pack_push(pack, bytes))
#define push_string(pack, str) (pack_push_str(pack, str))
#define push_data(pack, data, count, size) (pack_push_data(pack, data, count, size))

internal inline void *pack_pop(struct pack *pack, u32 id)
{
    // This pop nonsense just seems like a bad idea.. probably not resetting
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

global inline void *pack_lookup(pkid pkid)
{
    u32 pack_id = PKID_PACK(pkid);
    RICO_ASSERT(pack_id < MAX_PACKS);
    struct pack *pack = packs[pack_id];

    u32 blob_id = PKID_BLOB(pkid);
    RICO_ASSERT(blob_id < pack->blob_count);
    return pack_read(pack, pack->lookup[blob_id]);
}

#endif
