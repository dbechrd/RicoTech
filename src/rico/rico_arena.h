#ifndef RICO_ARENA_H
#define RICO_ARENA_H

static inline void ric_arena_alloc(struct ric_arena *arena, u32 bytes)
{
    DLB_ASSERT(bytes);
    arena->size = bytes;
    arena->buffer = calloc(1, bytes);
}

static inline void ric_arena_free(struct ric_arena *arena)
{
    arena->size = 0;
    arena->offset = 0;
    free(arena->buffer);
}

static inline void ric_arena_clear(struct ric_arena *arena)
{
    arena->offset = 0;
    memset(arena->buffer, 0, arena->size);
}

static inline void ric_arena_copy(struct ric_arena *dst, struct ric_arena *src)
{
    ric_arena_alloc(dst, src->size);
    DLB_ASSERT(dst->size == src->size);
    dst->offset = src->offset;
    memcpy(dst->buffer, src->buffer, dst->size);
    DLB_ASSERT(!memcmp(dst->buffer, src->buffer, dst->size));
}

static inline void *ric_arena_push(struct ric_arena *arena, u32 bytes)
{
    DLB_ASSERT(arena->offset + bytes <= arena->size);
    void *ptr = (u8 *)arena->buffer + arena->offset;
    arena->offset += bytes;
    return ptr;
}

#endif