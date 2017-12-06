#ifndef RICO_MESH_H
#define RICO_MESH_H

#define RICO_MESH_TYPES(f)  \
    f(MESH_OBJ_WORLD)       \
    f(MESH_STRING_SCREEN)

enum rico_mesh_type {
    RICO_MESH_TYPES(GEN_LIST)
};
extern const char *rico_mesh_type_string[];

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_mesh {
    struct hnd hnd;
    enum rico_mesh_type type;
    u32 ref_count;

    GLuint vao;
    GLuint vbos[2];
    GLsizei element_count;

    struct bbox bbox;
};
extern const u32 RICO_MESH_SIZE;
extern struct rico_mesh *RICO_DEFAULT_MESH;

struct rico_mesh *mesh_next(struct rico_mesh *mesh);
struct rico_mesh *mesh_prev(struct rico_mesh *mesh);
int mesh_init(struct rico_mesh *_mesh, const char *name,
              enum rico_mesh_type type, u32 vertex_count,
              const struct mesh_vertex *vertex_data, u32 element_count,
              const GLuint *element_data, GLenum hint);
void mesh_free(struct rico_mesh *mesh);
void mesh_update(struct rico_mesh *mesh);
void mesh_render(struct rico_mesh *mesh);

#endif // RICO_MESH_H
