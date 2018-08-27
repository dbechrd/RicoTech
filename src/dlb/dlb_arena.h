//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_ARENA_H
#define DLB_ARENA_H

#include "dlb_memory.h"

typedef struct Arena {
    char *ptr;      // Next free
    char *end;      // End of current block
    char **blocks;  // Linked list of allocated blocks
} Arena;

#define DLB_ARENA_ALIGNMENT 8
#define DLB_ARENA_BLOCK_SIZE 1024

void arena_grow(Arena *arena, size_t min_size);
void *arena_alloc(Arena *arena, size_t size);
void arena_free(Arena *arena);

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_ARENA_IMPLEMENTATION

void arena_grow(Arena *arena, size_t min_size) {
    size_t size = ALIGN_UP(MAX(DLB_ARENA_BLOCK_SIZE, min_size),
                           DLB_ARENA_ALIGNMENT);
    arena->ptr = xmalloc(size);
    arena->end = arena->ptr + size;
    buf_push(arena->blocks, arena->ptr);
}

void *arena_alloc(Arena *arena, size_t size) {
    // If current block isn't big enough, stop using it and allocate a new one
    if (size > (size_t)(arena->end - arena->ptr)) {
        arena_grow(arena, size);
        assert(size <= (size_t)(arena->end - arena->ptr));
    }
    void *ptr = arena->ptr;
    arena->ptr = ALIGN_UP_PTR(arena->ptr + size, DLB_ARENA_ALIGNMENT);
    assert(arena->ptr <= arena->end);
    assert(ptr == ALIGN_DOWN_PTR(ptr, DLB_ARENA_ALIGNMENT));
    return ptr;
}

void arena_free(Arena *arena) {
    for (char **it = arena->blocks; it != buf_end(arena->blocks); it++) {
        free(*it);
    }
}

#endif