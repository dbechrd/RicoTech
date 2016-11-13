#ifndef CONST_H
#define CONST_H

#define UNUSED(x) (void)(x)

// Cleanup: This is probably dumb
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

//------------------------------------------------------------------------------
// Basic type definitions
//------------------------------------------------------------------------------
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// TODO: Platform-specific definitions?
typedef unsigned char uchar;
typedef unsigned long ulong;

//------------------------------------------------------------------------------
// Rico constants
//------------------------------------------------------------------------------
// Debug
#ifndef RICO_DEBUG
#define RICO_DEBUG
#endif

#ifndef RICO_DEBUG_INFO
#define RICO_DEBUG_INFO
#endif

// Math / Physics
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

#define M_SEVENTH_DEG 51.428571428571428571428571428571

#ifndef EPSILON
#define EPSILON 0.001f
#endif

// Memory constants
#define RICO_TEXTURE_POOL_SIZE 50
#define RICO_MESH_POOL_SIZE 50
#define RICO_OBJECT_POOL_SIZE 50

#define RICO_SHADER_POS_LOC 0
#define RICO_SHADER_COL_LOC 1
#define RICO_SHADER_UV_LOC 2

//------------------------------------------------------------------------------
// Error codes
//------------------------------------------------------------------------------
//TODO: Replace with ERR enum values
#define ERR_GL_LINK_FAILED GL_FALSE
#define ERR_GL_ATTRIB_NOTFOUND -1

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define sizeof_member(type, member) sizeof(((type *)0)->member)

#ifdef RICO_DEBUG
#define rico_assert(exp) if(!(exp)) {*(int*)0=0;}
#else
#define rico_assert(exp)
#endif

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------
enum { VBO_VERTEX, VBO_ELEMENT };

enum {
    SUCCESS,
    ERR_BAD_ALLOC,
    ERR_POOL_OUT_OF_MEMORY,
    ERR_FILE_LOAD,
    ERR_TEXTURE_UNSUPPORTED_BPP,
    ERR_SHADER_COMPILE,
    ERR_SHADER_LINK
};

#endif // CONST_H