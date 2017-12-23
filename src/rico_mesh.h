#ifndef RICO_MESH_H
#define RICO_MESH_H

#define RICO_MESH_TYPES(f)  \
    f(MESH_OBJ_WORLD)       \
    f(MESH_STRING_SCREEN)

enum rico_mesh_type {
    RICO_MESH_TYPES(GEN_LIST)
};
extern const char *rico_mesh_type_string[];

struct rico_vertex {
    struct vec3 pos;
    struct col4 col;
    struct vec3 normal;
    struct tex2 uv;
};

struct rico_mesh {
    u32 id;
    u32 vertex_count;
    u32 element_count;
    struct bbox bbox;

    // TODO: Helper functions
    //const char *name;
    //struct rico_vertex *vertices;
    //u32 *elements;
    u32 name_offset;
    u32 vertices_offset;
    u32 elements_offset;

    // TODO: Store these in hash table when loaded, map UID -> gl id or
    //       track which meshes are currently loaded on GPU some other way.
    GLuint vao;
    GLuint vbos[2];
};

global const char *mesh_name(struct rico_mesh *mesh);
void mesh_upload(struct rico_mesh *mesh, GLenum hint);
void mesh_delete(struct rico_mesh *mesh);
void mesh_render(struct rico_mesh *mesh);

#endif // RICO_MESH_H
