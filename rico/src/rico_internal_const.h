#ifndef RICO_INTERNAL_CONST_H
#define RICO_INTERNAL_CONST_H

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
#define RICO_DEBUG_RUN_TESTS        RICO_DEBUG && 0
#define RICO_DEBUG_HND              RICO_DEBUG && 0
#define RICO_DEBUG_POOL             RICO_DEBUG && 0
#define RICO_DEBUG_CHUNK            RICO_DEBUG && 0
#define RICO_DEBUG_TEXTURE          RICO_DEBUG && 0
#define RICO_DEBUG_MESH             RICO_DEBUG && 0
#define RICO_DEBUG_OBJECT           RICO_DEBUG && 0
#define RICO_DEBUG_STRING           RICO_DEBUG && 0
#define RICO_DEBUG_MATERIAL         RICO_DEBUG && 0
#define RICO_DEBUG_FONT             RICO_DEBUG && 0
#define RICO_DEBUG_HASH             RICO_DEBUG && 0
#define RICO_DEBUG_PACK             RICO_DEBUG && 0
#define RICO_DEBUG_CAMERA           RICO_DEBUG && 1

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

//#define FLAG_PERSIST 31
//#define BIT_SET(num, bit, val) (num ^= (-val ^ num) & (1 << bit))
//#define BIT_GET(num, bit) ((num >> bit) & 1)
//#define FLAG_PERSIST_SET(handle, persist) BIT_SET(handle, FLAG_PERSIST, persist)
//#define FLAG_PERSIST_GET(handle)          BIT_GET(handle, FLAG_PERSIST)
#endif

//------------------------------------------------------------------------------
// Rico constants
//------------------------------------------------------------------------------
// Math / Physics
#define M_SEVENTH_DEG 51.428571428571428571428571428571
#define EPSILON 0.001f

#define SIM_MS ((r64)10)
#define SIM_SEC (SIM_MS / 1000)
#define SIM_MAX_FRAMESKIP_MS ((r64)50)
//#define SIM_MAX_FRAMESKIP_SEC (SIM_MAX_FRAMESKIP_MS / 1000)

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------
#define RICO_ERRORS(f)             \
    f(SUCCESS)                     \
    f(ERR_BAD_ALLOC)               \
    f(ERR_POOL_OUT_OF_MEMORY)      \
    f(ERR_POOL_INVALID_HANDLE)     \
    f(ERR_POOL_BAD_FREE)           \
    f(ERR_FILE_WRITE)              \
    f(ERR_FILE_READ)               \
    f(ERR_FILE_SIGNATURE)          \
    f(ERR_FILE_VERSION)            \
    f(ERR_SERIALIZE_DISABLED)      \
    f(ERR_SERIALIZER_NULL)         \
    f(ERR_DESERIALIZER_NULL)       \
    f(ERR_TEXTURE_LOAD)            \
    f(ERR_TEXTURE_UNSUPPORTED_BPP) \
    f(ERR_SHADER_COMPILE)          \
    f(ERR_SHADER_LINK)             \
    f(ERR_SDL_INIT)                \
    f(ERR_GL3W_INIT)               \
    f(ERR_OPENAL_INIT)             \
    f(ERR_PRIM_UNSUPPORTED)        \
    f(ERR_OBJ_TOO_MANY_VERTS)      \
    f(ERR_CHUNK_NULL)              \
    f(ERR_MESH_INVALID_NAME)       \
    f(ERR_TEXTURE_INVALID_NAME)    \
    f(ERR_MATERIAL_INVALID_NAME)   \
    f(ERR_OBJECT_INVALID_NAME)     \
    f(ERR_STRING_INVALID_NAME)     \
    f(ERR_FONT_INVALID_NAME)       \
    f(ERR_HASH_TABLE_FULL)         \
    f(ERR_HASH_INVALID_KEY)        \
    f(ERR_HASH_OVERWRITE)          \
    f(ERR_INVALID_PARAMS)          \
    f(ERR_CHUNK_FREE_FAILED)       \
    f(ERR_OBJ_PARSE_FAILED)

enum rico_error { RICO_ERRORS(GEN_LIST) };
const char *rico_error_string[];

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

#endif
