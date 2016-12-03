#include "rico_mesh.h"
#include "rico_texture.h"
#include "rico_pool.h"
#include <stdlib.h>

struct rico_mesh {
    struct rico_uid uid;

    GLuint vao;
    GLuint vbos[2];
    GLsizei element_count;

    struct bbox bbox;
};

uint32 RICO_MESH_DEFAULT = 0;
static struct rico_pool meshes;

static int build_mesh(struct rico_mesh *mesh, uint32 vertex_count,
                      const struct mesh_vertex *vertex_data,
                      uint32 element_count, const GLuint *element_data,
                      GLenum hint);

int rico_mesh_init(uint32 pool_size)
{
    return pool_init("Meshes", pool_size, sizeof(struct rico_mesh),
                     &meshes);
}

int mesh_load(const char *name, uint32 vertex_count,
              const struct mesh_vertex *vertex_data, uint32 element_count,
              const GLuint *element_data, GLenum hint, uint32 *_handle)
{
    enum rico_error err;
    *_handle = RICO_MESH_DEFAULT;

    err = pool_alloc(&meshes, _handle);
    if (err) return err;

    struct rico_mesh *mesh = pool_read(&meshes, *_handle);
    uid_init(&mesh->uid, RICO_UID_MESH, name);

    err = build_mesh(mesh, vertex_count, vertex_data, element_count,
                     element_data, hint);
    if (err) return err;

    err = bbox_init_mesh(&mesh->bbox, name, vertex_data, vertex_count,
                         COLOR_GRAY_HIGHLIGHT);
    return err;
}

static int build_mesh(struct rico_mesh *mesh, uint32 vertex_count,
                      const struct mesh_vertex *vertex_data,
                      uint32 element_count, const GLuint *element_data,
                      GLenum hint)
{
    mesh->element_count = element_count;

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(2, mesh->vbos);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(struct mesh_vertex),
                 vertex_data, hint);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbos[VBO_ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_count * sizeof(GLuint),
                 element_data, hint);

    //--------------------------------------------------------------------------
    // Get vertex shader attribute locations and set pointers (size/type/stride)
    //--------------------------------------------------------------------------

    // TODO: How to get size of mesh_vertex.pos dynamically? Why doesn't
    //       sizeof_member work?
    glVertexAttribPointer(RICO_SHADER_POS_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct mesh_vertex),
                          (GLvoid *)offsetof(struct mesh_vertex, pos));
    glEnableVertexAttribArray(RICO_SHADER_POS_LOC);

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

void mesh_free(uint32 handle)
{
    struct rico_mesh *mesh = pool_read(&meshes, handle);

    glDeleteBuffers(2, mesh->vbos);
    glDeleteVertexArrays(1, &mesh->vao);

    pool_free(&meshes, handle);
}

const char *mesh_name(uint32 handle)
{
    struct rico_mesh *mesh = pool_read(&meshes, handle);
    return mesh->uid.name;
}

const struct bbox *mesh_bbox(uint32 handle)
{
    struct rico_mesh *mesh = pool_read(&meshes, handle);
    return &mesh->bbox;
}

void mesh_update(uint32 handle)
{
    UNUSED(handle);
    //TODO: Animate the mesh.
}

void mesh_render(uint32 handle)
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