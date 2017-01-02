#include "rico_mesh.h"
#include "rico_texture.h"
#include "rico_pool.h"
#include <stdlib.h>

struct rico_mesh {
    struct rico_uid uid;
    u32 ref_count;

    GLuint vao;
    GLuint vbos[2];
    GLsizei element_count;

    struct bbox bbox;
};

u32 RICO_MESH_DEFAULT = 0;
static struct rico_pool meshes;

static int build_mesh(struct rico_mesh *mesh, u32 vertex_count,
                      const struct mesh_vertex *vertex_data,
                      u32 element_count, const GLuint *element_data,
                      GLenum hint);

int rico_mesh_init(u32 pool_size)
{
    return pool_init("Meshes", pool_size, sizeof(struct rico_mesh), 0, &meshes);
}

//TODO: Should be requesting const char* filename or u32 filename (string hash)
int mesh_request(u32 handle)
{
    struct rico_mesh *mesh = pool_read(&meshes, handle);
    mesh->ref_count++;
    return handle;
}

int mesh_load(const char *name, u32 vertex_count,
              const struct mesh_vertex *vertex_data, u32 element_count,
              const GLuint *element_data, GLenum hint, u32 *_handle)
{
#ifdef RICO_DEBUG_MESH
    printf("[Mesh] %s\nVertices: %d\n", name, vertex_count);
#endif

    enum rico_error err;
    *_handle = RICO_MESH_DEFAULT;

    err = pool_alloc(&meshes, _handle);
    if (err) return err;

    struct rico_mesh *mesh = pool_read(&meshes, *_handle);

    // Note: If we want to serialize mesh data we have to store the vertex data
    //       and element array in the struct.
    uid_init(&mesh->uid, RICO_UID_MESH, name, false);

    err = build_mesh(mesh, vertex_count, vertex_data, element_count,
                     element_data, hint);
    if (err) return err;

    err = bbox_init_mesh(&mesh->bbox, name, vertex_data, vertex_count,
                         COLOR_WHITE_HIGHLIGHT);
    return err;
}

static int build_mesh(struct rico_mesh *mesh, u32 vertex_count,
                      const struct mesh_vertex *vertex_data,
                      u32 element_count, const GLuint *element_data,
                      GLenum hint)
{
    mesh->element_count = element_count;

    //--------------------------------------------------------------------------
    // Generate VAO and buffers
    //--------------------------------------------------------------------------
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);
    glGenBuffers(2, mesh->vbos);

    //--------------------------------------------------------------------------
    // Vertex buffer
    //--------------------------------------------------------------------------
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(struct mesh_vertex),
                 vertex_data, hint);

    //--------------------------------------------------------------------------
    // Element buffer
    //--------------------------------------------------------------------------
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbos[VBO_ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_count * sizeof(GLuint),
                 element_data, hint);

    //--------------------------------------------------------------------------
    // Shader attribute pointers
    //--------------------------------------------------------------------------
    // TODO: How to get size of mesh_vertex.pos dynamically? Why doesn't
    //       sizeof_member work?
    glVertexAttribPointer(RICO_SHADER_POS_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct mesh_vertex),
                          (GLvoid *)offsetof(struct mesh_vertex, pos));
    glEnableVertexAttribArray(RICO_SHADER_POS_LOC);

    glVertexAttribPointer(RICO_SHADER_NORMAL_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct mesh_vertex),
                          (GLvoid *)offsetof(struct mesh_vertex, normal));
    glEnableVertexAttribArray(RICO_SHADER_NORMAL_LOC);

    glVertexAttribPointer(RICO_SHADER_COL_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct mesh_vertex),
                          (GLvoid *)offsetof(struct mesh_vertex, col));
    glEnableVertexAttribArray(RICO_SHADER_COL_LOC);

    glVertexAttribPointer(RICO_SHADER_UV_LOC, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct mesh_vertex),
                          (GLvoid *)offsetof(struct mesh_vertex, uv));
    glEnableVertexAttribArray(RICO_SHADER_UV_LOC);

    // Clean up
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return SUCCESS;
}

void mesh_free(u32 handle)
{
    struct rico_mesh *mesh = pool_read(&meshes, handle);


#ifdef RICO_DEBUG_MESH
    printf("[Mesh] Free %s\n", mesh->uid.name);
#endif

    mesh->ref_count--;
    if (mesh->ref_count > 0)
        return;

    bbox_free_mesh(&mesh->bbox);

    glDeleteBuffers(2, mesh->vbos);
    glDeleteVertexArrays(1, &mesh->vao);

    mesh->uid.uid = UID_NULL;
    pool_free(&meshes, handle);
}

const char *mesh_name(u32 handle)
{
    struct rico_mesh *mesh = pool_read(&meshes, handle);
    return mesh->uid.name;
}

const struct bbox *mesh_bbox(u32 handle)
{
    struct rico_mesh *mesh = pool_read(&meshes, handle);
    return &mesh->bbox;
}

void mesh_update(u32 handle)
{
    UNUSED(handle);
    //TODO: Animate the mesh.
}

void mesh_render(u32 handle)
{
    struct rico_mesh *mesh = pool_read(&meshes, handle);

    // Draw
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

struct rico_pool *mesh_pool_unsafe()
{
    return &meshes;
}