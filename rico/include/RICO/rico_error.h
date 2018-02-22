#ifndef RICO_ERROR_H
#define RICO_ERROR_H

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
    f(ERR_OBJ_PARSE_FAILED)        \
    f(ERR_COUNT)

enum RICO_error { RICO_ERRORS(GEN_LIST) };
extern const char *RICO_error_string[];

#endif