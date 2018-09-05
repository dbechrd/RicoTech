//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_HEAP_H
#define DLB_HEAP_H

#include "dlb_vector.h"

//typedef int (*dlb_heap_comparer)(void *a, void *b);

typedef struct dlb_heap_node {
    u32 priority;
    void *data;
} dlb_heap_node;

typedef struct dlb_heap {
    // Note: nodes[0] is reserved to make index arithmetic cleaner
    dlb_heap_node *nodes;
} dlb_heap;

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_HEAP_IMPLEMENTATION

void dlb_heap_init(dlb_heap *heap)
{
    dlb_vec_push(heap->nodes, (dlb_heap_node){ 0, 0 });
}

void dlb_heap_free(dlb_heap *heap)
{
    dlb_vec_free(heap->nodes);
}

size_t dlb_heap_size(dlb_heap *heap)
{
    size_t len = dlb_vec_len(heap->nodes);
    return len - 1;
}

bool dlb_heap_empty(dlb_heap *heap)
{
    size_t size = dlb_heap_size(heap);
    return size == 0;
}

void dlb_heap__swap_nodes(dlb_heap *heap, size_t a, size_t b)
{
    dlb_heap_node c = heap->nodes[a];
    heap->nodes[a] = heap->nodes[b];
    heap->nodes[b] = c;
}

#define dlb_heap__parent(index) ((index) / 2)

static size_t dlb_heap__left(dlb_heap *heap, size_t size, size_t index)
{
    size_t left = index * 2;
    return left < size ? left : 0;
}

static size_t dlb_heap__right(dlb_heap *heap, size_t size, size_t index)
{
    size_t right = index * 2 + 1;
    return right < size ? right : 0;
}

void dlb_heap__sift_up(dlb_heap *heap, size_t index)
{
    size_t parent = dlb_heap__parent(index);
    while (parent && heap->nodes[index].priority > heap->nodes[parent].priority)
    {
        dlb_heap__swap_nodes(heap, index, parent);
        index = parent;
        parent = dlb_heap__parent(index);
    }
}

void dlb_heap__sift_down(dlb_heap *heap, size_t index)
{
    size_t size = dlb_heap_size(heap);
    for (;;)
    {
        size_t left = dlb_heap__left(heap, size, index);
        size_t right = dlb_heap__right(heap, size, index);
        size_t swap = (heap->nodes[left].priority >= heap->nodes[right].priority)
            ? left : right;
        if (heap->nodes[index].priority >= heap->nodes[swap].priority)
            break;

        dlb_heap__swap_nodes(heap, index, swap);
        index = swap;
    }
}

void dlb_heap_push(dlb_heap *heap, u32 priority, void *data)
{
    DLB_ASSERT(priority > 0);
    dlb_vec_push(heap->nodes, (dlb_heap_node){ priority, data });
    dlb_heap__sift_up(heap, dlb_vec_len(heap->nodes) - 1);
}

void *dlb_heap_peek(dlb_heap *heap)
{
    if (dlb_heap_empty(heap))
    {
        return NULL;
    }
    void *data = heap->nodes[1].data;
    return data;
}

void *dlb_heap_pop(dlb_heap *heap)
{
    size_t last = dlb_heap_size(heap);
    if (!last)
    {
        return NULL;
    }

    void *data = heap->nodes[1].data;
    if (last >= 1)
    {
        if (last > 1)
        {
            heap->nodes[1] = heap->nodes[last];
            dlb_heap__sift_down(heap, 1);
        }
        dlb_vec_pop(heap->nodes);
    }
    return data;
}

#endif