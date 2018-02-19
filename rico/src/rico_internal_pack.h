#ifndef RICO_INTERNAL_PACK_BUILDER_H
#define RICO_INTERNAL_PACK_BUILDER_H

#include "RICO/rico_pack.h"

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

internal void *blob_start(struct pack *pack, enum rico_hnd_type type,
                          const char *name);

internal void pack_expand(struct pack *pack);
internal void pack_compact_buffer(struct pack *pack);

void *pack_next(pkid pkid, enum rico_hnd_type type);
void *pack_prev(pkid pkid, enum rico_hnd_type type);
void pack_delete(pkid pkid);

void pack_build_default();

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

#endif