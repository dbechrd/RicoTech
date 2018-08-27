//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_MEMORY_H
#define DLB_MEMORY_H

#include <stdlib.h>

void *dlb_malloc(size_t size);
void *dlb_calloc(size_t count, size_t size);
void *dlb_realloc(void *block, size_t size);

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_MEMORY_IMPLEMENTATION

void *dlb_malloc(size_t size)
{
    void *block = malloc(size);
    if (!block)
    {
        perror("xmalloc error");
        exit(1);
    }
    return block;
}

void *dlb_calloc(size_t count, size_t size)
{
    void *block = calloc(count, size);
    if (!block)
    {
        perror("xcalloc error");
        exit(1);
    }
    return block;
}

void *dlb_realloc(void *block, size_t size)
{
    block = realloc(block, size);
    if (!block)
    {
        perror("xrealloc error");
        exit(1);
    }
    return block;
}

#endif