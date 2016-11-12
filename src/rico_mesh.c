#include "rico_mesh.h"
#include "rico_texture.h"
#include "rico_pool.h"
#include "camera.h"
#include <stdlib.h>

struct rico_mesh {
    struct rico_uid uid;

    //TODO: Replace with program handle
    struct program_default *prog;

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

int mesh_load(const char *name, struct program_default *program,
              uint32 vertex_count, const struct mesh_vertex *vertex_data,
              uint32 element_count, const GLuint *element_data, GLenum hint,
              uint32 *_handle)
{
    int err;
    *_handle = RICO_MESH_DEFAULT;

    err = pool_alloc(&meshes, _handle);
    if (err) return err;

    struct rico_mesh *mesh = pool_read(&meshes, *_handle);
    uid_init(name, &mesh->uid);

    if (program == NULL)
    {
        err = make_program_default(&program);
        if (err) return err;
    }
    mesh->prog = program;
    
    // TODO: What am I doing with program ref counting? Should probably refcount
    //       program pool by uid, and check if already loaded when user requests
    //       a new program.. or something?
    //mesh->program->ref_count++;

    err = build_mesh(mesh, vertex_count, vertex_data, element_count,
                     element_data, hint);
    if (err) return err;

    err = bbox_init_mesh(&mesh->bbox, vertex_data, vertex_count,
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

    glUseProgram(mesh->prog->prog_id);

    //--------------------------------------------------------------------------
    // Get vertex shader attribute locations and set pointers (size/type/stride)
    //--------------------------------------------------------------------------
    if (mesh->prog->vert_pos >= 0)
    {
        // TODO: How to get size of mesh_vertex.pos dynamically? Why doesn't
        //       sizeof_member work?
        glVertexAttribPointer(mesh->prog->vert_pos, 4, GL_FLOAT, GL_FALSE,
                              sizeof(struct mesh_vertex),
                              (GLvoid *)offsetof(struct mesh_vertex, pos));
        glEnableVertexAttribArray(mesh->prog->vert_pos);
    }
    
    if (mesh->prog->vert_col >= 0)
    {
        glVertexAttribPointer(mesh->prog->vert_col, 4, GL_FLOAT, GL_FALSE,
                              sizeof(struct mesh_vertex),
                              (GLvoid *)offsetof(struct mesh_vertex, col));
        glEnableVertexAttribArray(mesh->prog->vert_col);
    }
    
    if (mesh->prog->vert_uv >= 0)
    {
        glVertexAttribPointer(mesh->prog->vert_uv, 2, GL_FLOAT, GL_FALSE,
                              sizeof(struct mesh_vertex),
                              (GLvoid *)offsetof(struct mesh_vertex, uv));
        glEnableVertexAttribArray(mesh->prog->vert_uv);
    }

    //--------------------------------------------------------------------------
    // Bind projection matrix
    //--------------------------------------------------------------------------
    glUniformMatrix4fv(mesh->prog->u_proj, 1, GL_TRUE, proj_matrix.a);

    glUseProgram(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //==========================================================================

    return SUCCESS;
}

void mesh_free(uint32 *handle)
{
    struct rico_mesh *mesh = pool_read(&meshes, *handle);

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

void mesh_render(uint32 handle, uint32 tex, const struct mat4 *model_matrix,
                 struct vec4 uv_scale)
{
    struct rico_mesh *mesh = pool_read(&meshes, handle);

    //if (view_polygon_mode != GL_FILL)
    //    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, view_polygon_mode);

    glUseProgram(mesh->prog->prog_id);

    glUniformMatrix4fv(mesh->prog->u_view, 1, GL_TRUE, view_matrix.a);
    glUniformMatrix4fv(mesh->prog->u_model, 1, GL_TRUE, model_matrix->a);

    // Model texture
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    glUniform1i(mesh->prog->u_tex, 0);

    // UV-coord scale
    glUniform2f(mesh->prog->u_scale_uv, uv_scale.x, uv_scale.y);
    
    // Bind texture(s)
    //glActiveTexture(GL_TEXTURE0);
    texture_bind(tex);

    // Draw
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
    texture_unbind(tex);

    //if (view_polygon_mode != GL_FILL)
    //    glPolygonMode(GL_FRONT_AND_BACK, view_polygon_mode);

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);
    //glDrawElements(
    //    GL_TRIANGLES,
    //    mesh->element_count,
    //    GL_UNSIGNED_SHORT,
    //    (void*)0
    //);*/
}
