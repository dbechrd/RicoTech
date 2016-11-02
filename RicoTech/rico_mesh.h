#ifndef MESH_H
#define MESH_H

#include "bbox.h"
#include "rico_uid.h"
#include <GL/gl3w.h>

struct rico_mesh {
    struct rico_uid uid;

    //TODO: Relace with program handle
    struct program_default *prog;

    GLuint vao;
    GLuint vbos[2];
    GLsizei element_count;
    
    //TODO: Replace with texture handle
    //TODO: Support multiple textures (per mesh?)
    const struct rico_texture *texture;

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
                            const struct rico_texture *texture,
                            const struct mesh_vertex *vertex_data,
                            uint32 vertex_count, const GLuint *element_data,
                            uint32 element_count, GLenum hint);
void free_mesh(struct rico_mesh **mesh);

void mesh_update(struct rico_mesh *mesh);
void mesh_render(const struct rico_mesh *mesh, const struct mat4 *model_matrix,
                 struct vec4 uv_scale);

#endif