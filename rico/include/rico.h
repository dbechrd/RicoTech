#ifndef RICO_H
#define RICO_H

#include "dlb_types.h"
#include "dlb_math.h"
#include "SDL/SDL.h"
#include "GL/gl3w.h"

//------------------------------------------------------------------------------
// const.c
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
// rico_hnd.c
//------------------------------------------------------------------------------
#define RICO_HND_TYPES(f)    \
    f(RICO_HND_NULL)         \
    f(RICO_HND_OBJECT)       \
    f(RICO_HND_TEXTURE)      \
    f(RICO_HND_MESH)         \
    f(RICO_HND_FONT)         \
    f(RICO_HND_STRING)       \
    f(RICO_HND_MATERIAL)     \
    f(RICO_HND_CEREAL_COUNT) \
    f(RICO_HND_BBOX)         \
    f(RICO_HND_CHUNK)        \
    f(RICO_HND_POOL)         \
    f(RICO_HND_HASHTABLE)

enum rico_hnd_type
{
    RICO_HND_TYPES(GEN_LIST)
};
const char *rico_hnd_type_string;

typedef u32 uid;
struct hnd
{
    uid uid;
    enum rico_hnd_type type;
    //struct rico_chunk *chunk;
    //struct rico_pool *pool;
    //struct pool_id id;
    u32 ref_count;
    char name[32];
    u32 len;
};
#define UID_NULL 0

//------------------------------------------------------------------------------
// bbox.c
//------------------------------------------------------------------------------
struct bbox
{
    struct vec3 min;
    struct vec3 max;
    bool selected;
};

//------------------------------------------------------------------------------
// rico_light.c
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
// rico_object.c
//------------------------------------------------------------------------------
#define RICO_OBJ_TYPES(f) \
    f(OBJ_NULL)           \
    f(OBJ_TERRAIN)        \
    f(OBJ_STATIC)         \
    f(OBJ_STRING_WORLD)   \
    f(OBJ_STRING_SCREEN)  \
    f(OBJ_LIGHT_POINT)

enum rico_obj_type
{
    RICO_OBJ_TYPES(GEN_LIST)
};
const char *rico_obj_type_string;

struct rico_transform
{
    struct vec3 trans;
    struct vec3 rot;
    struct vec3 scale;
    struct mat4 matrix;
    struct mat4 matrix_inverse;
};

#define RICO_PROP_TYPES(f) \
    f(PROP_NULL)           \
    f(PROP_MESH_ID)        \
    f(PROP_TEXTURE_ID)     \
    f(PROP_MATERIAL_ID)    \
    f(PROP_LIGHT_DIR)      \
    f(PROP_LIGHT_POINT)    \
    f(PROP_LIGHT_SPOT)     \
    f(PROP_LIGHT_SWITCH)   \
    f(PROP_AUDIO_SWITCH)   \
    f(PROP_GAME_BUTTON)    \
    f(PROP_COUNT)

enum obj_prop_type
{
    RICO_PROP_TYPES(GEN_LIST)
};
const char *rico_prop_type_string;

struct light_switch
{
    u32 light_id;
    bool state;
};

struct audio_switch
{
    u32 audio_id;
    bool state;
};

struct game_button
{
    u32 button_id;
    bool state;
};

struct obj_property
{
    enum obj_prop_type type;
    union
    {
        // PROP_MESH
        u32 mesh_id;
        // PROP_TEXTURE
        u32 texture_id;
        // PROP_MATERIAL
        u32 material_id;
        // PROP_LIGHT_DIR
        struct light_directional light_dir;
        // PROP_LIGHT_POINT
        struct light_point light_point;
        // PROP_LIGHT_SPOT
        struct light_spot light_spot;
        // PROP_LIGHT_SWITCH
        struct light_switch light_switch;
        // PROP_AUDIO_SWITCH
        struct audio_switch audio_switch;
        // PROP_GAME_BUTTON
        struct game_button game_button;
    };
};

//------------------------------------------------------------------------------
// rico_font.c
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
// rico_string.c
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
// pack_builder.c
//------------------------------------------------------------------------------
struct blob_index
{
    enum rico_hnd_type type;
    u32 offset;
    u32 size;
};

// Memory layout
// -----------------------
// struct pack;
// struct pack_entry index[index_count];
// struct pack_entry fast_index[index_count];
// u8 data[data_size];
struct pack
{
    char magic[4];
    u32 version;
    u32 id;
    char name[32];

    u32 blob_current_id;
    u32 blob_count;
    u32 blobs_used;

    u32 buffer_size;
    u32 buffer_used;

    u32 lookup_offset;
    u32 index_offset;
    u32 data_offset;

    u32 *lookup;
    struct blob_index *index;
    u8 *buffer;
};

#define MAX_PACKS 32
extern u32 packs_next;
extern struct pack *packs[MAX_PACKS];

struct pack *pack_init(u32 id, const char *name, u32 blob_count,
					   u32 buffer_size);
int pack_save(struct pack *pack, const char *filename, bool shrink);
int pack_load(const char *filename, struct pack **_pack);
void pack_free(u32 id);

u32 load_object(struct pack *pack, const char *name, enum rico_obj_type type,
				u32 prop_count, struct obj_property *props,
				const struct bbox *bbox);
u32 load_texture(struct pack *pack, const char *name, GLenum target, u32 width,
				 u32 height, u8 bpp, u8 *pixels);
u32 load_texture_file(struct pack *pack, const char *name,
					  const char *filename);
u32 load_texture_color(struct pack *pack, const char *name, struct vec4 color);
u32 load_material(struct pack *pack, const char *name, u32 tex0, u32 tex1,
				  u32 tex2);
u32 load_font(struct pack *pack, const char *name, const char *filename,
			  u32 *font_tex);
u32 load_mesh(struct pack *pack, const char *name, u32 vertex_size,
			  u32 vertex_count, const void *vertex_data, u32 element_count,
			  const GLuint *element_data);
u32 load_string(struct pack *pack, const char *name, enum rico_string_slot slot,
				float x, float y, struct vec4 color, u32 lifespan,
				struct rico_font *font, const char *text);
int load_obj_file(struct pack *pack, const char *filename, u32 *_last_mesh_id);

//------------------------------------------------------------------------------
// rico_internal.c
//------------------------------------------------------------------------------
int RIC_init(int argc, char* argv[]);
int RIC_run();
void RIC_quit();

#endif
