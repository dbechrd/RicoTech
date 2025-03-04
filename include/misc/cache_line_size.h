#ifndef CACHELINESIZE_H_INCLUDED
#define CACHELINESIZE_H_INCLUDED

// Author: Nick Strupat
// Date: October 29, 2010
// https://github.com/NickStrupat/CacheLineSize/blob/master/CacheLineSize.h
// Returns the cache line size (in bytes) of the processor, or 0 on failure

#include <stddef.h>
size_t CacheLineSize();

#endif