#include "rico_mesh.h"
#include "rico_texture.h"
#include "camera.h"
#include <stdlib.h>

struct rico_mesh *make_mesh(const char *name, struct program_default *program,
                            const struct rico_texture *texture,
                            const struct mesh_vertex *vertex_data,
                            uint32 vertex_count, const GLuint *element_data,
                            uint32 element_count, GLenum hint)
{
    if (program == NULL)
        program = make_program_default();
    if (texture == NULL)
        texture = RICO_TEXTURE_DEFAULT;

    struct rico_mesh *mesh = calloc(1, sizeof(*mesh));
    uid_init(name, &mesh->uid);

    program->ref_count++;
    mesh->prog = program;
    mesh->texture = texture;
    mesh->element_count = element_count;

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(2, mesh->vbos);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(struct mesh_vertex),
                 vertex_data, hint);

    GLuint elements[6] = { 0, 1, 3, 1, 2, 3 };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbos[VBO_ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_count * sizeof(GLuint),
                 element_data, GL_STATIC_DRAW);

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

    bbox_init_mesh(&(mesh->bbox), vertex_data, vertex_count,
                   COLOR_GRAY_HIGHLIGHT);

    return mesh;
}

void free_mesh(struct rico_mesh **mesh)
{
    (*mesh)->prog->ref_count--;
    glDeleteBuffers(2, (*mesh)->vbos);
    glDeleteVertexArrays(1, &(*mesh)->vao);
    free(*mesh);
    *mesh = NULL;
}

void mesh_update(struct rico_mesh *mesh)
{
    (void)mesh;
    //TODO: Animate the mesh.
}

void mesh_render(const struct rico_mesh *mesh, const struct mat4 *model_matrix,
                 struct vec4 uv_scale)
{
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
    texture_bind(mesh->texture);

    // Draw
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
    texture_unbind(mesh->texture);

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
