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
#ifndef RICO_DEBUG_INFO
    #define RICO_DEBUG_INFO
    #define RICO_DEBUG_UID
    #define RICO_DEBUG_POOL
    #define RICO_DEBUG_CHUNK
#endif
#endif

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
enum { VBO_VERTEX, VBO_ELEMENT };

#define RICO_ERRORS(f) \
    f(SUCCESS)                      \
    f(ERR_BAD_ALLOC)                \
    f(ERR_POOL_OUT_OF_MEMORY)       \
    f(ERR_FILE_WRITE)               \
    f(ERR_FILE_READ)                \
    f(ERR_FILE_SIGNATURE)           \
    f(ERR_SERIALIZER_NULL)          \
    f(ERR_DESERIALIZER_NULL)        \
    f(ERR_TEXTURE_LOAD)             \
    f(ERR_TEXTURE_UNSUPPORTED_BPP)  \
    f(ERR_SHADER_COMPILE)           \
    f(ERR_SHADER_LINK)              \
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

#endif // CONST_H