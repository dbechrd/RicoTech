#ifndef MESH_H
#define MESH_H

#include "bbox.h"
#include "rico_uid.h"
#include "rico_texture.h"
#include <GL/gl3w.h>

struct mesh_vertex {
    struct vec4 pos;
    struct col4 col;
    struct tex2 uv;
    //struct vec4 normal;
    //GLfloat shininess;
    //GLubyte specular[4];
};

extern uint32 RICO_MESH_DEFAULT;

int rico_mesh_init(uint32 pool_size);
int mesh_load(const char *name, uint32 vertex_count,
              const struct mesh_vertex *vertex_data, uint32 element_count,
              const GLuint *element_data, GLenum hint, uint32 *_handle);
void mesh_free(uint32 *handle);
const char *mesh_name(uint32 handle);
const struct bbox *mesh_bbox(uint32 handle);
void mesh_update(uint32 handle);
void mesh_render(uint32 handle);

#endif