#ifndef MESH_H
#define MESH_H

#include "bbox.h"
#include "rico_uid.h"
#include "rico_texture.h"
#include <GL/gl3w.h>

/*
// Cleanup: Do I need static slots here? Anywhere?
#define RICO_MESH_SLOTS(f)    \
    f(MESH_SLOT_NULL)         \
    f(MESH_SLOT_SELECTED_OBJ) \
    f(MESH_SLOT_EDIT_INFO)    \
    f(MESH_SLOT_FPS)          \
    f(MESH_SLOT_COUNT)        \
    f(MESH_SLOT_DYNAMIC)

enum rico_mesh_slot {
    RICO_MESH_SLOTS(GEN_LIST)
};
extern const char *rico_mesh_slot_string[];
*/

#define RICO_MESH_TYPES(f)  \
    f(MESH_NULL)            \
    f(MESH_OBJ_WORLD)          \
    f(MESH_STRING_SCREEN)

enum rico_mesh_type {
    RICO_MESH_TYPES(GEN_LIST)
};
extern const char *rico_mesh_type_string[];

struct mesh_vertex {
    struct vec3 pos;
    struct col4 col;
    struct vec3 normal;
    struct tex2 uv;
    //GLfloat shininess;
    //GLubyte specular[4];
};

extern u32 RICO_MESH_DEFAULT;

int rico_mesh_init(u32 pool_size);
u32 mesh_request(u32 handle);
u32 mesh_request_by_name(const char *name);
u32 mesh_next(u32 handle);
u32 mesh_prev(u32 handle);
int mesh_load(u32 *_handle, const char *name, enum rico_mesh_type type,
              u32 vertex_count, const struct mesh_vertex *vertex_data,
              u32 element_count, const GLuint *element_data, GLenum hint);
void mesh_free(u32 handle);
const char *mesh_name(u32 handle);
const struct bbox *mesh_bbox(u32 handle);
void mesh_update(u32 handle);
void mesh_render(u32 handle);

struct rico_pool *mesh_pool_unsafe();

#endif