#ifndef CONST_H
#define CONST_H

// Debug
#ifndef RICO_DEBUG
    #define RICO_DEBUG
#endif

#ifdef RICO_DEBUG
    #ifndef RICO_DEBUG_ERROR_ASSERT
        #define RICO_DEBUG_ERROR_ASSERT
    #endif
    #ifndef RICO_DEBUG_WARN
        #define RICO_DEBUG_WARN
    #endif
    #ifndef RICO_DEBUG_INFO
        #define RICO_DEBUG_INFO
        #define RICO_DEBUG_UID
        // #define RICO_DEBUG_POOL
        #define RICO_DEBUG_CHUNK
        #define RICO_DEBUG_TEXTURE
        #define RICO_DEBUG_MESH
        #define RICO_DEBUG_OBJECT
        #define RICO_DEBUG_STRING
        #define RICO_DEBUG_MATERIAL
    #endif
#endif

//------------------------------------------------------------------------------
// Rico constants
//------------------------------------------------------------------------------
// Memory constants
#define RICO_TEXTURE_POOL_SIZE 100
#define RICO_MATERIAL_POOL_SIZE 50
#define RICO_MESH_POOL_SIZE 100
#define RICO_OBJECT_POOL_SIZE 100
#define RICO_FONT_POOL_SIZE 10
#define RICO_STRING_POOL_SIZE 16

#define RICO_SHADER_POS_LOC 0
#define RICO_SHADER_COL_LOC 1
#define RICO_SHADER_NORMAL_LOC 2
#define RICO_SHADER_UV_LOC 3

// Math / Physics
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

#define DEG_TO_RAD(deg) deg * M_PI / 180.0
#define RAD_TO_DEG(rad) rad * 180.0 / M_PI

#define DEG_TO_RADF(deg) deg * (float)M_PI / 180.0f
#define RAD_TO_DEGF(rad) rad * 180.0f / (float)M_PI

#define M_SEVENTH_DEG 51.428571428571428571428571428571

#ifndef EPSILON
#define EPSILON 0.001f
#endif

//------------------------------------------------------------------------------
// Basic type redefinitions
//------------------------------------------------------------------------------
#include <stdint.h>
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

typedef u32 bool;
#define true 1
#define false 0

/*
//
// NOTE(casey): Types
//
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;
typedef b32 b32x;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;
typedef real32 f32;
typedef real64 f64;
*/

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define sizeof_member(type, member) sizeof(((type *)0)->member)

// Used to generate enums
#define GEN_LIST(val) val,
#define GEN_STRING(val) #val,

#define UNUSED(x) (void)(x)

#ifdef RICO_DEBUG
    #define RICO_ASSERT(exp) if(!(exp)) {*(int*)0=0;}
#else
    #define RICO_ASSERT(exp)
#endif

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------
enum rico_vbo {
    VBO_VERTEX,
    VBO_ELEMENT,
    VBO_COUNT
};

//------------------------------------------------------------------------------
#define RICO_ERRORS(f)              \
    f(SUCCESS)                      \
    f(ERR_BAD_ALLOC)                \
    f(ERR_POOL_OUT_OF_MEMORY)       \
    f(ERR_FILE_WRITE)               \
    f(ERR_FILE_READ)                \
    f(ERR_FILE_SIGNATURE)           \
    f(ERR_SERIALIZE_DISABLED)       \
    f(ERR_SERIALIZER_NULL)          \
    f(ERR_DESERIALIZER_NULL)        \
    f(ERR_TEXTURE_LOAD)             \
    f(ERR_TEXTURE_UNSUPPORTED_BPP)  \
    f(ERR_SHADER_COMPILE)           \
    f(ERR_SHADER_LINK)              \
    f(ERR_SDL_INIT)                 \
    f(ERR_PRIM_UNSUPPORTED)         \
    f(ERR_OBJ_TOO_MANY_VERTS)       \
    f(ERR_CHUNK_NULL)               \
    f(ERR_COUNT)

enum rico_error {
    RICO_ERRORS(GEN_LIST)
};
extern const char *rico_error_string[];

#ifdef RICO_DEBUG
    enum rico_error rico_error_print(enum rico_error err, const char *file,
                                     int line);
    #define RICO_ERROR(err) rico_error_print(err, __FILE__, __LINE__)
#else
    #define RICO_ERROR(err) err
#endif

//------------------------------------------------------------------------------

#endif // CONST_H