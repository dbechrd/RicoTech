#ifndef RICO_INTERNAL_MESH_H
#define RICO_INTERNAL_MESH_H

#include "RICO/rico_mesh.h"

enum rico_vbo
{
    VBO_VERTEX,
    VBO_ELEMENT,
    VBO_COUNT
};

// NOTE: Must fit in hash value
struct rgl_mesh
{
    GLuint vao;
    GLuint vbos[2];
    u32 vertices;
    u32 elements;
};

static void *mesh_vertices(struct rico_mesh *mesh);
static void mesh_upload(struct rico_mesh *mesh, GLenum hint,
                 enum program_type prog_type);
static void mesh_delete(struct rico_mesh *mesh);
static void mesh_render(pkid pkid, enum program_type prog_type);

#endif
