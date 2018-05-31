#ifndef RICO_INTERNAL_PACK_BUILDER_H
#define RICO_INTERNAL_PACK_BUILDER_H

#include "rico_pack.h"

#define PACK_ALLOC_ZERO_MEMORY
#define MAX_PACKS 32
struct pack *packs[MAX_PACKS];
u32 packs_next;

static void pack_delete(pkid pkid);
static void pack_build_default();

static inline void *pack_push(struct pack *pack, u32 bytes)
{
    RICO_ASSERT(pack->buffer_used + bytes < pack->buffer_size);
    void *ptr = pack->buffer + pack->buffer_used;
#ifdef PACK_ALLOC_ZERO_MEMORY
    memset(ptr, 0, bytes); // Re-zero memory
#endif
    pack->buffer_used += bytes;
    pack->index[pack->lookup[pack->blob_current_id]].size += bytes;
    return ptr;
}
static inline void *pack_push_data(struct pack *pack, const void *data,
                                     u32 count, u32 size)
{
    u32 bytes = count * size;
    void *ptr = pack_push(pack, bytes);
    memcpy(ptr, data, bytes);
    return ptr;
}
static inline void *pack_push_str(struct pack *pack, const char *str)
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

static inline void *pack_pop(struct pack *pack, u32 id)
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

static inline void *pack_read(struct pack *pack, u32 index)
{
    //RICO_ASSERT(index > 0);  // Cleanup: Disallow reading null blob
    RICO_ASSERT(index < pack->blobs_used);
    RICO_ASSERT(pack->index[index].type);
    return pack->buffer + pack->index[index].offset;
}

#endif