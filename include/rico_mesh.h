#ifndef MESH_H
#define MESH_H

#include "bbox.h"
#include "rico_uid.h"
#include "rico_texture.h"
#include <GL/gl3w.h>

struct rico_mesh {
    struct rico_uid uid;

    //TODO: Replace with program handle
    struct program_default *prog;

    GLuint vao;
    GLuint vbos[2];
    GLsizei element_count;

    struct bbox bbox;
};

struct mesh_vertex {
    struct vec4 pos;
    struct col4 col;
    struct tex2 uv;
    //struct vec4 normal;
    //GLfloat shininess;
    //GLubyte specular[4];
};

struct rico_mesh *make_mesh(const char *name, struct program_default *program,
                            uint32 vertex_count,
                            const struct mesh_vertex *vertex_data,
                            uint32 element_count,
                            const GLuint *element_data,
                            GLenum hint);
void free_mesh(struct rico_mesh **mesh);
void mesh_update(struct rico_mesh *mesh);
void mesh_render(const struct rico_mesh *mesh, uint32 tex,
                 const struct mat4 *model_matrix, struct vec4 uv_scale);

#endif