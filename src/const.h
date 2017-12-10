#ifndef CONST_H
#define CONST_H

//------------------------------------------------------------------------------
// Start-up
//------------------------------------------------------------------------------
// Open GL
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 2

// Save file
#define RICO_SAVE_BACKUP 0

// Debug
#define RICO_DEBUG 1
#define RICO_DEBUG_FATAL_ASSERT     RICO_DEBUG && 0
#define RICO_DEBUG_ALL_ERRORS_FATAL RICO_DEBUG && 1
#define RICO_DEBUG_WARN             RICO_DEBUG && 1
#define RICO_DEBUG_INFO             RICO_DEBUG && 1
#define RICO_DEBUG_HND              RICO_DEBUG && 0
#define RICO_DEBUG_POOL             RICO_DEBUG && 1
#define RICO_DEBUG_CHUNK            RICO_DEBUG && 0
#define RICO_DEBUG_TEXTURE          RICO_DEBUG && 0
#define RICO_DEBUG_MESH             RICO_DEBUG && 1
#define RICO_DEBUG_OBJECT           RICO_DEBUG && 1
#define RICO_DEBUG_STRING           RICO_DEBUG && 1
#define RICO_DEBUG_MATERIAL         RICO_DEBUG && 0
#define RICO_DEBUG_FONT             RICO_DEBUG && 0
#define RICO_DEBUG_HASH             RICO_DEBUG && 0

// TODO: Get rid of this crap, use realloc when necessary
// Memory pools
//#define RICO_POOL_SIZE_OBJECT           128
//#define RICO_POOL_SIZE_TEXTURE          128
//#define RICO_POOL_SIZE_MESH             128
//#define RICO_POOL_SIZE_BBOX             128
//#define RICO_POOL_SIZE_FONT             4
//#define RICO_POOL_SIZE_STRING           32
//#define RICO_POOL_SIZE_MATERIAL         128

#if 0
// Handle section flags
// ------------------------------------------------------
// | persist | middle             | value               |
// |       0 | 000 0000 0000 0000 | 0000 0000 0000 0000 |
// ------------------------------------------------------
#define FLAG_PERSIST 0x80000000  // 31
#define FLAG_MIDDLE  0x7FFF0000  // 15
#define FLAG_HANDLE  0x0000FFFF  // 0

// Handle section get/set
#define HANDLE_PERSIST(handle) (enum rico_persist)((handle & FLAG_PERSIST) >> 31)
#define HANDLE_MIDDLE(handle)  ((handle & FLAG_MIDDLE) >> 15)
#define HANDLE_VALUE(handle)   ((handle & FLAG_HANDLE) >> 0)
#define HANDLE_PERSIST_SET(handle, val) ((handle & ~FLAG_PERSIST) & (val << 31))
#define HANDLE_MIDDLE_SET(handle, val)  ((handle & ~FLAG_MIDDLE) & (val << 15))
#define HANDLE_VALUE_SET(handle, val)   ((handle & ~FLAG_HANDLE) & (val << 0))
#endif

//#define FLAG_PERSIST 31
//#define BIT_SET(num, bit, val) (num ^= (-val ^ num) & (1 << bit))
//#define BIT_GET(num, bit) ((num >> bit) & 1)
//#define FLAG_PERSIST_SET(handle, persist) BIT_SET(handle, FLAG_PERSIST, persist)
//#define FLAG_PERSIST_GET(handle)          BIT_GET(handle, FLAG_PERSIST)

//------------------------------------------------------------------------------
// Rico constants
//------------------------------------------------------------------------------
// TODO: Is there a better way to handle this?
#define RICO_SHADER_POS_LOC     0
#define RICO_SHADER_COL_LOC     1
#define RICO_SHADER_NORMAL_LOC  2
#define RICO_SHADER_UV_LOC      3

// Math / Physics
#define M_PI 3.14159265358979323846264338327950288
#define M_2PI 6.28318530717958647692528676655900576

#define DEG_TO_RAD(deg) deg * M_PI / 180.0
#define RAD_TO_DEG(rad) rad * 180.0 / M_PI

#define DEG_TO_RADF(deg) deg * (float)M_PI / 180.0f
#define RAD_TO_DEGF(rad) rad * 180.0f / (float)M_PI

#define LERP(v0, v1, t) (1 - t) * v0 + t * v1

#define M_SEVENTH_DEG 51.428571428571428571428571428571

#define EPSILON 0.001f

//------------------------------------------------------------------------------
// Basic type redefinitions
//------------------------------------------------------------------------------
//#include <stdint.h>
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
#define global   static

/*
//
// NOTE(casey): Types
//
//#include <stdint.h>
//#include <stddef.h>
//#include <limits.h>
//#include <float.h>

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
// Enums
//------------------------------------------------------------------------------
// Used to generate enums
#define GEN_LIST(e, ...) e,
#define GEN_STRING(e, ...) #e,
#define GEN_VALUE(e, str) str,

enum rico_vbo {
    VBO_VERTEX,
    VBO_ELEMENT,
    VBO_COUNT
};

#define RICO_ERRORS(f)              \
    f(SUCCESS)                      \
    f(ERR_BAD_ALLOC)                \
    f(ERR_POOL_OUT_OF_MEMORY)       \
    f(ERR_POOL_INVALID_HANDLE)      \
    f(ERR_FILE_WRITE)               \
    f(ERR_FILE_READ)                \
    f(ERR_FILE_SIGNATURE)           \
    f(ERR_FILE_VERSION)             \
    f(ERR_SERIALIZE_DISABLED)       \
    f(ERR_SERIALIZER_NULL)          \
    f(ERR_DESERIALIZER_NULL)        \
    f(ERR_TEXTURE_LOAD)             \
    f(ERR_TEXTURE_UNSUPPORTED_BPP)  \
    f(ERR_SHADER_COMPILE)           \
    f(ERR_SHADER_LINK)              \
    f(ERR_SDL_INIT)                 \
    f(ERR_GL3W_INIT)                \
    f(ERR_PRIM_UNSUPPORTED)         \
    f(ERR_OBJ_TOO_MANY_VERTS)       \
    f(ERR_CHUNK_NULL)               \
    f(ERR_MESH_INVALID_NAME)        \
    f(ERR_TEXTURE_INVALID_NAME)     \
    f(ERR_MATERIAL_INVALID_NAME)    \
    f(ERR_OBJECT_INVALID_NAME)      \
    f(ERR_STRING_INVALID_NAME)      \
    f(ERR_FONT_INVALID_NAME)        \
    f(ERR_HASH_TABLE_FULL)          \
    f(ERR_HASH_INVALID_KEY)         \
    f(ERR_INVALID_PARAMS)           \
    f(ERR_OBJ_PARSE_FAILED)

enum rico_error {
    RICO_ERRORS(GEN_LIST)
};
//extern const char *rico_error_string[];

//------------------------------------------------------------------------------
// Global functions
//------------------------------------------------------------------------------
// TODO: Move this to rico_error.h?
//enum rico_error rico_error_print(enum rico_error err, const char *desc,
//                                 const char *file, int line);
//enum rico_error rico_fatal_print(enum rico_error err, const char *desc,
//                                 const char *file, int line);
enum rico_error rico_error_print(const char *file, int line,
                                 enum rico_error err, const char *fmt, ...);
enum rico_error rico_fatal_print(const char *file, int line,
                                 enum rico_error err, const char *fmt, ...);

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define UNUSED(x) ((void)sizeof(x))
#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#define sizeof_member(type, member) sizeof(((type *)0)->member)
#define HALT() SDL_TriggerBreakpoint()

#if RICO_DEBUG
    #define FILE_LOC __FILE__, __LINE__
    #define RICO_ASSERT(exp) if(!(exp)) HALT();
    #define RICO_FATAL(err, desc, ...) rico_fatal_print(FILE_LOC, err, desc, ##__VA_ARGS__)

    #if RICO_DEBUG_ALL_ERRORS_FATAL
      #define RICO_ERROR(err, desc, ...) RICO_FATAL(err, desc, ##__VA_ARGS__)
    #else
      #define RICO_ERROR(err, desc, ...) rico_error_print(FILE_LOC, err, desc)
    #endif
#else
    #define RICO_ASSERT(exp) UNUSED(exp)
    #define RICO_FATAL(err, desc, ...) err
    #define RICO_ERROR(err, desc, ...) err
#endif

//------------------------------------------------------------------------------

#endif // CONST_H
