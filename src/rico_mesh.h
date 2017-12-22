#ifndef RICO_MESH_H
#define RICO_MESH_H

#define RICO_MESH_TYPES(f)  \
    f(MESH_OBJ_WORLD)       \
    f(MESH_STRING_SCREEN)

enum rico_mesh_type {
    RICO_MESH_TYPES(GEN_LIST)
};
extern const char *rico_mesh_type_string[];

struct rico_mesh {
    struct hnd hnd;
    enum rico_mesh_type type;

    GLuint vao;
    GLuint vbos[2];

    GLsizei vertex_count;
    GLsizei element_count;
    struct mesh_vertex *vertices;
    u32 *elements;

    struct bbox bbox;
};
extern struct pool_id RICO_DEFAULT_MESH;

int mesh_init(struct rico_mesh *mesh, const char *name,
              enum rico_mesh_type type, u32 vertex_count,
              const struct mesh_vertex *vertex_data, u32 element_count,
              const GLuint *element_data, GLenum hint);
int mesh_free(struct rico_mesh *mesh);
void mesh_update(struct rico_mesh *mesh);
void mesh_render(struct rico_mesh *mesh);

#endif // RICO_MESH_H
