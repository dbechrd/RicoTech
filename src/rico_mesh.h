#ifndef RICO_MESH_H
#define RICO_MESH_H

#define RICO_MESH_TYPES(f)  \
    f(MESH_NULL)            \
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

/*
// Cleanup: Do I need fixed slots here? Anywhere?
#define RICO_MESH_SLOTS(f)    \
    f(MESH_SLOT_NULL)         \
    f(MESH_SLOT_SELECTED_OBJ) \
    f(MESH_SLOT_EDIT_INFO)    \
    f(MESH_SLOT_FPS)          \
    f(MESH_SLOT_COUNT)        \
    f(MESH_SLOT_DYNAMIC)

enum rico_mesh_slot {
    RICO_MESH_SLOTS(GEN_LIST)
};
extern const char *rico_mesh_slot_string[];
*/

extern struct hnd *RICO_DEFAULT_MESH;

struct rico_mesh *mesh_request(struct rico_mesh *mesh);
int mesh_request_by_name(struct rico_mesh *_mesh, const char *name);
struct rico_mesh *mesh_next(struct rico_mesh *mesh);
struct rico_mesh *mesh_prev(struct rico_mesh *mesh);
int mesh_load(struct rico_mesh *_mesh, enum rico_persist persist, const char *name,
              enum rico_mesh_type type, u32 vertex_count,
              const struct mesh_vertex *vertex_data, u32 element_count,
              const GLuint *element_data, GLenum hint);
void mesh_free(struct rico_mesh *mesh);
const char *mesh_name(struct rico_mesh *mesh);
const struct bbox *mesh_bbox(struct rico_mesh *mesh);
void mesh_update(struct rico_mesh *mesh);
void mesh_render(struct rico_mesh *mesh);

#endif // RICO_MESH_H
