#ifndef RICO_MESH_H
#define RICO_MESH_H

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

struct rico_mesh
{
    u32 id;
    u32 vertex_size;
    u32 vertex_count;
    u32 element_count;
    struct bbox bbox;

    u32 name_offset;
    u32 vertices_offset;
    u32 elements_offset;
};

global const char *mesh_name(struct rico_mesh *mesh);
global void *mesh_vertices(struct rico_mesh *mesh);
void mesh_upload(struct rico_mesh *mesh, GLenum hint,
                 enum program_type prog_type);
void mesh_delete(struct rico_mesh *mesh);
void mesh_render(struct pack *pack, u32 id, enum program_type prog_type);

#endif
