#ifndef MESH_H
#define MESH_H

#include "bbox.h"
#include "rico_uid.h"
#include "rico_texture.h"
#include <GL/gl3w.h>

struct mesh_vertex {
    struct vec3 pos;
    struct col4 col;
    struct tex2 uv;
    //struct vec3 normal;
    //GLfloat shininess;
    //GLubyte specular[4];
};

extern u32 RICO_MESH_DEFAULT;

int rico_mesh_init(u32 pool_size);
int mesh_request(u32 handle);
int mesh_load(const char *name, u32 vertex_count,
              const struct mesh_vertex *vertex_data, u32 element_count,
              const GLuint *element_data, GLenum hint, u32 *_handle);
void mesh_free(u32 handle);
const char *mesh_name(u32 handle);
const struct bbox *mesh_bbox(u32 handle);
void mesh_update(u32 handle);
void mesh_render(u32 handle);

struct rico_pool *mesh_pool_unsafe();

#endif