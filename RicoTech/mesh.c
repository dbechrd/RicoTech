#include "mesh.h"
#include "bbox.h"
#include "camera.h"
#include <stdlib.h>

struct rico_mesh *make_mesh(struct program_default *program,
                            const struct rico_texture *texture,
                            const struct mesh_vertex *vertex_data,
                            uint32 vertex_count, const GLuint *element_data,
                            uint32 element_count, GLenum hint)
{
    struct rico_mesh *mesh = calloc(1, sizeof(struct rico_mesh));

    program->ref_count++;
    mesh->prog = program;
    mesh->texture = texture;

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(2, mesh->vbos);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, hint);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbos[VBO_ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element_data), element_data,
                 GL_STATIC_DRAW);

    //==============================================================================

    //--------------------------------------------------------------------------
    // Get vertex shader attribute locations and set pointers (size/type/stride)
    //--------------------------------------------------------------------------
    if (mesh->prog->vert_pos >= 0)
    {
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

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //==============================================================================

    mesh->bbox = make_bbox_mesh(vertex_data, vertex_count);
    bbox_init(mesh->bbox);

    return mesh;
}

void free_mesh(struct rico_mesh **mesh)
{
    (*mesh)->prog->ref_count--;
    glDeleteBuffers(2, (*mesh)->vbos);
    glDeleteVertexArrays(1, &(*mesh)->vao);
    free(*mesh);
    (*mesh) = NULL;
}

void mesh_update(struct rico_mesh *mesh)
{
    (void)mesh;
    //TODO: Animate the mesh.
}

void mesh_render(const struct rico_mesh *mesh)
{
    glUseProgram(mesh->prog->prog_id);

    glUniformMatrix4fv(mesh->prog->u_view, 1, GL_TRUE, view_matrix.a);

    // Model transform
    struct mat4 model_matrix;
    mat4_ident(&model_matrix);
    
    //HACK: Order of these operations might not always be the same.. should
    //      probably just store the transformation matrix directly rather than
    //      trying to figure out which order to do what.
    mat4_translate(&model_matrix, mesh->trans);
    mat4_rotx(&model_matrix, mesh->rot.x);
    mat4_roty(&model_matrix, mesh->rot.y);
    mat4_rotz(&model_matrix, mesh->rot.z);
    mat4_scale(&model_matrix, mesh->scale);
    glUniformMatrix4fv(mesh->prog->u_model, 1, GL_TRUE, model_matrix.a);

    // Model texture
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    //glUniform1i(mesh->prog->u_tex, 0);

    // UV-coord scale
    struct tex2 uv_scale = (struct tex2) { 128.0f, 128.0f };
    glUniform2f(mesh->prog->u_scale_uv, uv_scale.u, uv_scale.v);
    
    // Bind texture(s)
    //glActiveTexture(GL_TEXTURE0);
    //texture_bind(mesh->texture);
    glBindTexture(GL_TEXTURE_2D, mesh->texture->texture_id);

    // Draw
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
    //texture_unbind(mesh->texture);
    glBindTexture(GL_TEXTURE_2D, 0);

    bbox_render_color(mesh->bbox, &model_matrix, COLOR_CYAN);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);
    //glDrawElements(
    //    GL_TRIANGLES,
    //    mesh->element_count,
    //    GL_UNSIGNED_SHORT,
    //    (void*)0
    //);*/
}
