//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_VECTOR_H
#define DLB_VECTOR_H

#include <stdlib.h>
#include "dlb_types.h"
#include "dlb_memory.h"

typedef struct dlb_vec__hdr {
    size_t len;
    size_t cap;
    char buf[0];
} dlb_vec__hdr;

#define dlb_vec__hdr(b) ((dlb_vec__hdr *)((char *)b - offsetof(dlb_vec__hdr, buf)))

#define dlb_vec_len(b) ((b) ? dlb_vec__hdr(b)->len : 0)
#define dlb_vec_cap(b) ((b) ? dlb_vec__hdr(b)->cap : 0)
#define dlb_vec_end(b) ((b) + dlb_vec_len(b))
//#define dlb_vec_sizeof(b) ((b) ? dlb_vec_len(b) * sizeof(*(b)) : 0)
#define dlb_vec_reserve(b, n) \
    ((n) <= dlb_vec_cap(b) ? 0 : ((b) = dlb_vec__grow((b), (n), sizeof(*(b)))))
#define dlb_vec_push(b, ...) \
    (dlb_vec_reserve((b), 1 + dlb_vec_len(b)), \
                     (b)[dlb_vec__hdr(b)->len++] = (__VA_ARGS__))
#define dlb_vec_free(b) ((b) ? (free(dlb_vec__hdr(b)), (b) = NULL) : 0)

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#define DLB_VECTOR_IMPLEMENTATION
#ifdef DLB_VECTOR_IMPLEMENTATION

void *dlb_vec__grow(const void *buf, size_t len, size_t size) {
    DLB_ASSERT(dlb_vec_cap(buf) <= (SIZE_MAX - 1) / 2);
    size_t capacity = MAX(16, MAX(1 + 2 * dlb_vec_cap(buf), len));
    DLB_ASSERT(len <= capacity);
    DLB_ASSERT(capacity <= (SIZE_MAX - offsetof(dlb_vec__hdr, buf))/size);
    size_t new_size = offsetof(dlb_vec__hdr, buf) + capacity * size;
    dlb_vec__hdr *vec;
    if (buf) {
        vec = dlb_realloc(dlb_vec__hdr(buf), new_size);
    } else {
        vec = dlb_malloc(new_size);
        vec->len = 0;
    }
    vec->cap = capacity;
    return vec->buf;
}

#endif
