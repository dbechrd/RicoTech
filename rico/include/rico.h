#ifndef RICO_H
#define RICO_H

#include "dlb_types.h"
#include "dlb_math.h"
#include "SDL/SDL.h"
#include "GL/gl3w.h"

//------------------------------------------------------------------------------
// const
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

enum rico_error
{
    RICO_ERRORS(GEN_LIST)
};
//const char *rico_error_string[];

//------------------------------------------------------------------------------
// rico_hnd
//------------------------------------------------------------------------------
#include "../src/rico_hnd.h"

//------------------------------------------------------------------------------
// bbox
//------------------------------------------------------------------------------
struct bbox
{
    struct vec3 min;
    struct vec3 max;
    bool selected;
};

//------------------------------------------------------------------------------
// rico_light
//------------------------------------------------------------------------------
struct light_ambient
{
    struct vec3 color;
};

struct light_directional
{
    struct vec3 color;
    struct vec3 direction;
};

struct light_point
{
    struct vec3 color;
    struct vec3 position;
    float intensity;

    // Distance fall-off
    float kc;  // Constant
    float kl;  // Linear
    float kq;  // Quadratic
};

struct light_spot
{
    struct vec3 color;
    struct vec3 position;
    struct vec3 direction;

    // Angle of inner (full intensity) and outer (fall-off to zero) cone
    float theta_inner;
    float theta_outer;

    // Distance fall-off
    float kc;  // Constant
    float kl;  // Linear
    float kq;  // Quadratic
};

//------------------------------------------------------------------------------
// rico_object
//------------------------------------------------------------------------------
#include "../src/rico_object.h"

//------------------------------------------------------------------------------
// rico_font
//------------------------------------------------------------------------------
struct rico_font
{
    u32 id;
    u32 cell_x;
    u32 cell_y;
    u8 base_char;
    u32 row_pitch;
    float col_factor;
    float row_factor;
    s32 y_offset;
    bool y_invert;
    u8 render_style;
    u8 char_widths[256];
    u32 tex_id;

    u32 name_offset;
};

//------------------------------------------------------------------------------
// rico_string
//------------------------------------------------------------------------------
#define RICO_STRING_SLOTS(f) \
    f(STR_SLOT_DEBUG)        \
    f(STR_SLOT_SELECTED_OBJ) \
    f(STR_SLOT_STATE)        \
    f(STR_SLOT_FPS)          \
    f(STR_SLOT_MENU_QUIT)    \
    f(STR_SLOT_DELTA)        \
    f(STR_SLOT_WIDGET)       \
    f(STR_SLOT_DYNAMIC)
enum rico_string_slot
{
    RICO_STRING_SLOTS(GEN_LIST)
};
const char *rico_string_slot_string;

struct rico_string
{
    u32 id;
    enum rico_string_slot slot;
    u32 object_id;
    u32 lifespan;
};

//------------------------------------------------------------------------------
// pack_builder
//------------------------------------------------------------------------------
#include "../src/pack_builder.h"

//------------------------------------------------------------------------------
// rico_internal
//------------------------------------------------------------------------------
int RIC_init(int argc, char* argv[]);
int RIC_run();
void RIC_quit();

#endif
