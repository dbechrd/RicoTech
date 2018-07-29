#ifndef RICO_INTERNAL_H
#define RICO_INTERNAL_H

#if defined(__GNUC__) || defined(__clang__)
#include <x86intrin.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "GL/gl3w.h"
#include "SDL/SDL.h"
//#undef main
#include "AL/al.h"
#include "AL/alc.h"
#include "MurmurHash3.h"

#include "dlb_types.h"
#include "dlb_string.h"
#include "dlb_math.h"
#include "dlb_hash.h"

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
#define RICO_DEBUG_RUN_TESTS        RICO_DEBUG && 1
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
#define HANDLE_PERSIST(handle) \
    (enum rico_persist)((handle & FLAG_PERSIST) >> 31)
#define HANDLE_MIDDLE(handle)  ((handle & FLAG_MIDDLE) >> 15)
#define HANDLE_VALUE(handle)   ((handle & FLAG_HANDLE) >> 0)
#define HANDLE_PERSIST_SET(handle, val) ((handle & ~FLAG_PERSIST) & (val << 31))
#define HANDLE_MIDDLE_SET(handle, val)  ((handle & ~FLAG_MIDDLE) & (val << 15))
#define HANDLE_VALUE_SET(handle, val)   ((handle & ~FLAG_HANDLE) & (val << 0))

//#define FLAG_PERSIST 31
//#define BIT_SET(num, bit, val) (num ^= (-val ^ num) & (1 << bit))
//#define BIT_GET(num, bit) ((num >> bit) & 1)
//#define FLAG_PERSIST_SET(hnd, persist) BIT_SET(hnd, FLAG_PERSIST, persist)
//#define FLAG_PERSIST_GET(hnd)          BIT_GET(hnd, FLAG_PERSIST)
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
// Macros
//------------------------------------------------------------------------------
// TODO: Replace these with the stuff in dlb_types
#define HALT() SDL_TriggerBreakpoint()

#if RICO_DEBUG
#  define FILE_LOC __FILE__, __LINE__
#  define IF_DEBUG(exp) exp
#  define RICO_ASSERT(exp) if(!(exp)) HALT();
#  define RICO_FATAL(err, desc, ...) \
          rico_fatal_print(FILE_LOC, err, desc, ##__VA_ARGS__)
#
#  if RICO_DEBUG_ALL_ERRORS_FATAL
#    define RICO_ERROR(err, desc, ...) RICO_FATAL(err, desc, ##__VA_ARGS__)
#  else
#    define RICO_ERROR(err, desc, ...) rico_error_print(FILE_LOC, err, desc)
#  endif
#else
#  define IF_DEBUG(exp) UNUSED(exp)
#  define RICO_ASSERT(exp) UNUSED(exp)
#  define RICO_FATAL(err, desc, ...) err
#  define RICO_ERROR(err, desc, ...) err
#endif

#include "ri_program.h"

#ifdef __APPLE__
#define __gl_h_
//#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

//#include "CacheLineSize.h"
//#include "nuklear.h"
//#include "nuklear_sdl_gl3.h"

#define STBI_ONLY_TGA
#include "stb_image.h"
#include "tinyobjloader.h"
#include "MurmurHash3.h"

///////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////
extern struct pbr_program *prog_pbr;
extern struct shadow_texture_program *prog_shadow_texture;
extern struct shadow_cubemap_program *prog_shadow_cubemap;
extern struct primitive_program *prog_prim;
extern struct text_program *prog_text;

extern struct dlb_hash global_fonts;
extern struct dlb_hash global_materials;
extern struct dlb_hash global_meshes;
extern struct dlb_hash global_textures;

#define PACK_ALLOC_ZERO_MEMORY
#define MAX_PACKS 32
extern struct pack *packs[MAX_PACKS];
extern u32 packs_next;

extern float trans_delta;

extern pkid global_string_slots[STR_SLOT_COUNT + 64];

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
static void window_render();

static void init_openal();

struct RICO_mesh;
static void bbox_init(struct RICO_bbox *bbox, struct vec3 min, struct vec3 max);
static void bbox_init_mesh(struct RICO_bbox *bbox, struct RICO_mesh *mesh);

static void camera_init(struct RICO_camera *camera, struct vec3 position,
                        struct quat view, float fov_deg);
static void camera_reset(struct RICO_camera *camera);
static void camera_set_fov(struct RICO_camera *camera, float fov_deg);
static void camera_toggle_projection(struct RICO_camera *camera);
static void camera_translate_world(struct RICO_camera *camera,
                                   const struct vec3 *v);
static void camera_translate_local(struct RICO_camera *camera,
                                   const struct vec3 *v);
static void camera_rotate(struct RICO_camera *camera, float dx, float dy,
                          float dz);
static void camera_update(struct RICO_camera *camera, r64 sim_alpha);
static void camera_player_update(struct RICO_camera *camera, s32 dx, s32 dy,
                                 struct vec3 delta_acc, float sim_alpha);
static void camera_render(struct RICO_camera *camera);
static void camera_fwd_ray(struct ray *_ray, struct RICO_camera *camera);

static bool collide_ray_plane(struct vec3 *_contact, const struct ray *ray,
                              const struct vec3 *p, const struct vec3 *n);
static bool collide_ray_bbox(float *_t, const struct ray *ray,
                             const struct RICO_bbox *RICO_bbox,
                             const struct mat4 *transform);
static bool collide_ray_obb(float *_dist, const struct ray *r,
                            const struct RICO_bbox *RICO_bbox,
                            const struct mat4 *model_matrix);

static int rico_convert(int argc, char **argv);
static int rico_convert_obj(const char *filename);
static int load_obj_file_new(const char *filename);

#if 0
static inline char *read_word(char *fp, int buffer_len, char **buf)
{
    buffer_len--;
    while (*fp && buffer_len)
    {
        (*buf)++;
    }
    **buf = '\0';

    return fp;
}
#endif

static void editor_init();
static void edit_object_create();
static void edit_bbox_reset_all();
static void edit_object_select(pkid id, bool force);
static void edit_object_next();
static void edit_object_prev();
static void edit_print_object();
static void edit_translate(struct RICO_camera *camera,
                           const struct vec3 *offset);
static void edit_rotate(const struct quat *offset);
static void edit_scale(const struct vec3 *offset);
static void edit_mesh_next();
static void edit_mesh_prev();
static void edit_bbox_reset();
static void edit_duplicate();
static void edit_delete();
static void edit_mouse_pressed();
static void edit_mouse_move();
static void edit_mouse_released();
static void edit_render();
static void free_glref();

static enum RICO_error rico_error_print(const char *file, int line,
                                        enum RICO_error err, const char *fmt,
                                        ...);
static enum RICO_error rico_fatal_print(const char *file, int line,
                                        enum RICO_error err, const char *fmt,
                                        ...);

static int rico_file_open_write(struct rico_file *_handle, const char *filename,
                                u32 version);
static int rico_file_open_read(struct rico_file *_handle, const char *filename);
static void rico_file_close(struct rico_file *handle);

static void font_render(pkid *mesh_id, pkid *tex_id, pkid font_id, float x,
                        float y, struct vec4 bg, const char *text,
                        const char *mesh_name);

static void rico_resource_init();

int rico_heiro_init();
int heiro_load_glyphs(FT_Face ft_face);
int heiro_load_glyph(u8 **buffer, struct RICO_heiro_glyph *glyph,
                     FT_Face ft_face, FT_ULong char_code);
GLuint heiro_upload_texture(s32 width, s32 height, const void *pixels);
void heiro_delete_glyphs();
void heiro_delete_glyph(struct RICO_heiro_glyph *glyph);
void heiro_free();

static void material_bind(pkid pkid);
static void material_unbind(pkid pkid);

static void rico_mesh_init();
static void *mesh_vertices(struct RICO_mesh *mesh);
static void mesh_upload(struct RICO_mesh *mesh, GLenum hint);
static void mesh_delete(struct RICO_mesh *mesh);
static void mesh_bind_buffers(pkid pkid);
static GLuint mesh_vao(pkid pkid);
static void mesh_render(pkid pkid);
static void mesh_clusterfuck(pkid pkid);

static void object_delete(struct RICO_object *obj);
static struct RICO_object *object_copy(u32 pack, struct RICO_object *other,
                                       const char *name);
static void object_update_colliders(struct RICO_object *obj);
static void object_bbox_recalculate_all(u32 id);
static void object_transform_update(struct RICO_object *obj);
static void object_rot(struct RICO_object *obj, const struct quat *q);
static void object_rot_set(struct RICO_object *obj, const struct quat *q);
static const struct quat *object_rot_get(struct RICO_object *obj);
static void object_scale(struct RICO_object *obj, const struct vec3 *v);
static void object_scale_set(struct RICO_object *obj, const struct vec3 *v);
static const struct vec3 *object_scale_get(struct RICO_object *obj);
static const struct mat4 *object_matrix_get(struct RICO_object *obj);
static bool object_collide_ray(float *_dist, struct RICO_object *obj,
                               const struct ray *ray);
static bool object_collide_ray_type(pkid *_object_id, float *_dist,
                                    const struct ray *ray);
static void object_render_all(r64 alpha, struct RICO_camera *camera);
static void object_render(struct pack *pack, GLint model_location, bool shadow);
static void object_print(struct RICO_object *obj);

static void pack_delete(pkid pkid);
static void pack_build_default();

static inline void *pack_push(struct pack *pack, u32 bytes)
{
    RICO_ASSERT(pack->buffer_used + bytes < pack->buffer_size);
    void *ptr = pack->buffer + pack->buffer_used;
#ifdef PACK_ALLOC_ZERO_MEMORY
    memset(ptr, 0, bytes); // Re-zero memory
#endif
    pack->buffer_used += bytes;
    pack->index[pack->lookup[pack->blob_current_id]].min_size += bytes;
    return ptr;
}
static inline void *pack_push_data(struct pack *pack, const void *data,
                                   u32 bucket_count, u32 min_size)
{
    u32 bytes = bucket_count * min_size;
    void *ptr = pack_push(pack, bytes);
    memcpy(ptr, data, bytes);
    return ptr;
}
static inline void *pack_push_str(struct pack *pack, const char *str)
{
    u32 min_size = (u32)strlen(str) + 1;
    void *ptr = pack_push(pack, min_size);
    memcpy(ptr, str, min_size);
    return ptr;
}
#define push_struct(pack, type) ((type *)pack_push(pack, sizeof(type)))
#define push_bytes(pack, bytes) (pack_push(pack, bytes))
#define push_string(pack, str) (pack_push_str(pack, str))
#define push_data(pack, data, bucket_count, min_size) \
    (pack_push_data(pack, data, bucket_count, min_size))

static inline void *pack_pop(struct pack *pack, u32 id)
{
    // This pop nonsense just seems like a bad idea.. probably not resetting
    // index_iter properly, who knows what other side effects.
    RICO_ASSERT(0);
    UNUSED(pack);
    UNUSED(id);
    return NULL;

#if 0
    u32 offset = pack->index[pack->lookup[id]].offset;
    RICO_ASSERT(offset <= pack->buffer_used);

    if (offset < pack->buffer_used)
    {
        memset(pack->buffer + offset, 0, pack->buffer_used - offset);
        pack->buffer_used = offset;
    }
    void *ptr = pack->buffer + pack->buffer_used;
    return ptr;
#endif
}

static inline void *pack_read(struct pack *pack, u32 index)
{
    //RICO_ASSERT(index > 0);  // Cleanup: Disallow reading null blob
    RICO_ASSERT(index < pack->blobs_used);
    RICO_ASSERT(pack->index[index].type);
    return pack->buffer + pack->index[index].offset;
}

static struct rico_physics *make_physics(struct vec3 min_size);
static void free_physics(struct rico_physics *phys);
static void update_physics(struct rico_physics *phys, int bucket_count);

static int prim_init();
static void prim_free();
static void prim_draw_line(const struct vec3 *p0, const struct vec3 *p1,
                           const struct vec4 *color, const struct mat4 *xform,
                           const struct mat4 *view, const struct mat4 *proj);
static void prim_draw_quad(u32 vertex_count, const struct prim_vertex *vertices,
                           const struct vec4 *color, const struct mat4 *xform,
                           const struct mat4 *view, const struct mat4 *proj,
                           pkid tex_id);

/*************************************************************************
| Shader types:
|
| GL_VERTEX_SHADER      Vertex shader.
| GL_GEOMETRY_SHADER    Geometry shader.
| GL_FRAGMENT_SHADER    Fragment shader.
|
*************************************************************************/
static int make_shader(const GLenum type, const char *filename,
                       GLuint *_shader);

static inline void free_shader(GLuint shader)
{
    if (shader) glDeleteShader(shader);
}

static void rico_check_key_events();

static void string_delete(struct RICO_string *str);
static void string_free_slot(enum RICO_string_slot slot);
static void string_update();
static void string_render(struct RICO_string *str, GLint model_location);
static void string_render_all(GLint model_location);

// TODO: Rename "static void rico_" to "static void ri_"
static void rico_texture_init();
static void texture_delete(struct RICO_texture *texture);
static void texture_bind(pkid pkid, GLenum texture_unit);
static void texture_unbind(pkid pkid, GLenum texture_unit);

static void rico_ui_init();
static void rico_ui_reset();
static void ui_draw_element(struct RICO_ui_element *element, s32 x, s32 y);

//int file_contents(const char *filename, int *_length, char **_buffer);
////void *read_tga(const char *filename, int *width, int *height);
//
//void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id,
//                                     GLenum severity, GLsizei length,
//                                     const GLchar *message,
//                                     const void *userParam);
//
//void show_info_log(GLuint object,
//                   PFNGLGETSHADERIVPROC glGet__iv,
//                   PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

static void string_truncate(char *buf, int buf_count, int length);
static int file_contents(const char *filename, u32 *_length, char **_buffer);

static inline int str_starts_with(const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

#endif