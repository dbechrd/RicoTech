#ifndef RICO_MESH_H
#define RICO_MESH_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_mesh {
    struct rico_uid uid;
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

#define RICO_MESH_TYPES(f)  \
    f(MESH_NULL)            \
    f(MESH_OBJ_WORLD)       \
    f(MESH_STRING_SCREEN)

enum rico_mesh_type {
    RICO_MESH_TYPES(GEN_LIST)
};
extern const char *rico_mesh_type_string[];

struct mesh_vertex {
    struct vec3 pos;
    struct col4 col;
    struct vec3 normal;
    struct tex2 uv;
    //GLfloat shininess;
    //GLubyte specular[4];
};

extern hash_key RICO_DEFAULT_MESH;

u32 mesh_request(u32 handle);
int mesh_request_by_key(u32 *_handle, hash_key key);
int mesh_request_by_name(u32 *_handle, const char *name);
u32 mesh_next(u32 handle);
u32 mesh_prev(u32 handle);
int mesh_load(hash_key *_key, const char *name, enum rico_mesh_type type,
              u32 vertex_count, const struct mesh_vertex *vertex_data,
              u32 element_count, const GLuint *element_data, GLenum hint);
void mesh_free(u32 handle);
const char *mesh_name(u32 handle);
const struct bbox *mesh_bbox(u32 handle);
void mesh_update(u32 handle);
void mesh_render(u32 handle);

#endif // RICO_MESH_H