#ifndef RICO_INTERNAL_MESH_H
#define RICO_INTERNAL_MESH_H

#include "rico_mesh.h"

enum rico_vbo
{
    VBO_VERTEX,
    VBO_ELEMENT,
    VBO_COUNT
};

struct rgl_mesh
{
    GLuint vao;
    GLuint vbos[2];
    u32 vertices;
    u32 elements;
    struct rgl_mesh *next;
};

extern void rico_mesh_init();
static void *mesh_vertices(struct RICO_mesh *mesh);
static void mesh_upload(struct RICO_mesh *mesh, GLenum hint,
                        enum program_type prog_type);
static void mesh_delete(struct RICO_mesh *mesh);
static void mesh_render(pkid pkid, enum program_type prog_type);

#endif