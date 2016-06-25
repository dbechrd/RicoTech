#ifndef MESH_H
#define MESH_H

#include "geom.h"
#include <GL/gl3w.h>

struct mesh {
    GLsizei element_count;

    GLuint vertex_buffer;
    GLuint element_buffer;
    
    GLuint texture;
    GLuint vao;
};

struct mesh_vertex {
    struct vec4 position;
    struct vec4 normal;
    struct tex2 tex;
    //GLfloat shininess;
    //GLubyte specular[4];
};

struct mesh *make_mesh(const struct mesh_vertex *vertex_data,
                       GLsizei vertex_count,
                       const GLushort *element_data,
                       GLsizei element_count,
                       GLenum hint);
void free_mesh(struct mesh *);

void update_mesh(struct mesh *);
void render_mesh(struct mesh *);

#endif