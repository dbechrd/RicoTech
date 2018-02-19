#ifndef DLB_TYPES_H
#define DLB_TYPES_H

//------------------------------------------------------------------------------
// Basic type redefinitions
//------------------------------------------------------------------------------
#include <stdint.h>
#include <assert.h>
// #include <stddef.h>
// #include <limits.h>
// #include <float.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;

typedef u32 bool;
#define true 1
#define false 0

// NOTE: internal and global are relative to translation unit
#define internal static
#define local    static
#if 0
#define global   static
#endif

// Enums generators
#define GEN_LIST(e, ...) e,
#define GEN_STRING(e, ...) #e,
#define GEN_VALUE(e, str) str,

// Useful macros
#define UNUSED(x) ((void)x)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))
#define SIZEOF_MEMBER(type, member) sizeof(((type *)0)->member)

#endif