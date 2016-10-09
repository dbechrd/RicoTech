#include "const.h"
#include "geom.h"
#include "program.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Bounding box
////////////////////////////////////////////////////////////////////////////////

mat4 view_matrix = NULL;

enum { VBO_VERTEX, VBO_ELEMENT };

void bbox_init(struct bbox *box)
{
    glGenVertexArrays(1, &box->vao);
    glBindVertexArray(box->vao);

    glGenBuffers(2, box->vbos);

    glBindBuffer(GL_ARRAY_BUFFER, box->vbos[VBO_VERTEX]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(box->vertices), box->vertices,
                 GL_STATIC_DRAW);

    //--------------------------------------------------------------------------
    // Bind element data to buffer
    //--------------------------------------------------------------------------

    // Bind element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box->vbos[VBO_ELEMENT]);

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
                              4 * sizeof(GL_FLOAT),
                              (GLvoid *)(0));
        glEnableVertexAttribArray(box->program->vert_pos);
    }

    //--------------------------------------------------------------------------
    // Bind projection matrix
    //--------------------------------------------------------------------------
    mat4 proj_matrix = make_mat4_perspective(SCREEN_W, SCREEN_H,
                                             Z_NEAR, Z_FAR, Z_FOV_DEG);

    glUniformMatrix4fv(box->program->u_proj, 1, GL_FALSE, proj_matrix);

    free_mat4(&proj_matrix);

    glUseProgram(0);
    //--------------------------------------------------------------------------

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void bbox_render(const struct bbox *box, mat4 model_matrix)
{
    bbox_render_color(box, model_matrix, box->color);
}

void bbox_render_color(const struct bbox *box, mat4 model_matrix,
                       const struct col4 color)
{
    //--------------------------------------------------------------------------
    // Draw
    //--------------------------------------------------------------------------
    glUseProgram(box->program->prog_id);

    glUniformMatrix4fv(box->program->u_view, 1, GL_FALSE, view_matrix);

    // Model transform
    //model_matrix = make_mat4_ident();
    //mat4_scale(model_matrix, (struct vec4) { 10.0f, 10.0f, 10.0f });
    //mat4_translate(model_matrix, (struct vec4) { 0.0f, 0.0f, 0.0f });
    //mat4_roty(model_matrix, 30.0f);
    glUniformMatrix4fv(box->program->u_model, 1, GL_FALSE, model_matrix);
    //free_mat4(&model_matrix);

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

    // BBox color
    glUniform4f(box->program->u_color, color.r, color.g, color.b, color.a);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Draw
    glBindVertexArray(box->vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
    //glBindTexture(tex_default->target, 0);

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}