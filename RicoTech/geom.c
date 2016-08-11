#include "const.h"
#include "geom.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Bounding box
////////////////////////////////////////////////////////////////////////////////

static GLuint vao;
static GLuint vbos[2];

enum { VBO_VERTEX, VBO_ELEMENT };

void bbox_init(const struct bbox *box)
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(2, vbos);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBO_VERTEX]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(box->vertices), box->vertices,
                 GL_STATIC_DRAW);

    //--------------------------------------------------------------------------
    // Bind element data to buffer
    //--------------------------------------------------------------------------

    // Bind element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[VBO_ELEMENT]);

    // Bbox faces
    //GLuint elements[36] = {
    //    0, 1, 2, 2, 3, 0,
    //    4, 0, 3, 3, 7, 4,
    //    5, 4, 7, 7, 6, 5,
    //    1, 5, 6, 6, 2, 1,
    //    3, 2, 6, 6, 7, 3,
    //    0, 1, 5, 5, 4, 0
    //};
    GLuint elements[36] = {
        0, 1, 2, 2, 3, 0,
        4, 0, 3, 3, 7, 4,
        5, 4, 7, 7, 6, 5,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
                 GL_STATIC_DRAW);

    //--------------------------------------------------------------------------
     glUseProgram(box->program->prog_id);

    //--------------------------------------------------------------------------
    // Get vertex shader attribute locations and set pointers (size/type/stride)
    //--------------------------------------------------------------------------

    if (box->program->vert_pos >= 0)
    {
        glVertexAttribPointer(box->program->vert_pos, 4, GL_FLOAT, GL_FALSE,
                              10 * sizeof(GL_FLOAT),
                              (GLvoid *)(0));
        glEnableVertexAttribArray(box->program->vert_pos);
    }

    if (box->program->vert_col >= 0)
    {
        glVertexAttribPointer(box->program->vert_col, 4, GL_FLOAT, GL_FALSE,
                              10 * sizeof(GL_FLOAT),
                              (GLvoid *)(4 * sizeof(GL_FLOAT)));
        glEnableVertexAttribArray(box->program->vert_col);
    }

    if (box->program->vert_uv >= 0)
    {
        glVertexAttribPointer(box->program->vert_uv, 2, GL_FLOAT, GL_FALSE,
                              10 * sizeof(GL_FLOAT),
                              (GLvoid *)(8 * sizeof(GL_FLOAT)));
        glEnableVertexAttribArray(box->program->vert_uv);
    }

    glUseProgram(0);
    //--------------------------------------------------------------------------

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void bbox_render(const struct bbox *box, mat5 model_matrix)
{
    //--------------------------------------------------------------------------
    // Draw
    //--------------------------------------------------------------------------

    //mat5 model_matrix;
    //struct tex2 uv_scale;

    //TODO: Don't have multiple textures / model matrices for single mesh
    //      Let the mesh render itself, set uniforms in mesh_init()

    //--------------------------------------------------------------------------
    // Hello object
    //--------------------------------------------------------------------------

    glUseProgram(box->program->prog_id);

    // Model transform
    //model_matrix = make_mat5_ident();
    //mat5_scale(model_matrix, (struct vec4) { 10.0f, 10.0f, 10.0f });
    //mat5_translate(model_matrix, (struct vec4) { 0.0f, 0.0f, 0.0f });
    //mat5_roty(model_matrix, 30.0f);
    glUniformMatrix4fv(box->program->u_model, 1, GL_FALSE, model_matrix);
    //free_mat5(&model_matrix);

    // Model texture
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    //glUniform1i(box->program->u_tex, 0);

    // UV-coord scale
    //uv_scale = (struct tex2) { 1.0f, 1.0f };
    //glUniform2f(box->program->u_scale_uv, uv_scale.u, uv_scale.v);

    // Bind texture(s)
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(target, texture);

    // Draw
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
    //glBindTexture(tex_default->target, 0);

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}