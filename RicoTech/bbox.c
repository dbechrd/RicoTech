#include "bbox.h"
#include "const.h"
#include "camera.h"
#include "geom.h"

#ifndef BBOX_EPSILON
#define BBOX_EPSILON 0.1f
#endif

static void bbox_init(struct bbox *box);

const struct bbox *make_bbox(struct vec4 p0, struct vec4 p1)
{
    return make_bbox_color(p0, p1, COLOR_WHITE);
}

const struct bbox *make_bbox_color(struct vec4 p0, struct vec4 p1, struct col4 color)
{
    struct bbox *bbox = (struct bbox *)calloc(1, sizeof(struct bbox));
    bbox->prog = make_program_bbox();
    bbox->vertices[0] = (struct vec4) { p0.x, p0.y, p0.z, 1.0f };
    bbox->vertices[1] = (struct vec4) { p1.x, p0.y, p0.z, 1.0f };
    bbox->vertices[2] = (struct vec4) { p1.x, p1.y, p0.z, 1.0f };
    bbox->vertices[3] = (struct vec4) { p0.x, p1.y, p0.z, 1.0f };
    bbox->vertices[4] = (struct vec4) { p0.x, p0.y, p1.z, 1.0f };
    bbox->vertices[5] = (struct vec4) { p1.x, p0.y, p1.z, 1.0f };
    bbox->vertices[6] = (struct vec4) { p1.x, p1.y, p1.z, 1.0f };
    bbox->vertices[7] = (struct vec4) { p0.x, p1.y, p1.z, 1.0f };
    bbox->color = color;

    bbox_init(bbox);
    return bbox;
}

const struct bbox *make_bbox_mesh(const struct mesh_vertex *verts, int count)
{
    struct vec4 p0 = (struct vec4) { 9999.0f, 9999.0f, 9999.0f };
    struct vec4 p1 = (struct vec4) { -9999.0f, -9999.0f, -9999.0f };

    // Find bounds of mesh
    for (int i = 0; i < count; ++i)
    {
        if (verts[i].pos.x < p0.x)
            p0.x = verts[i].pos.x;
        else if (verts[i].pos.x > p1.x)
            p1.x = verts[i].pos.x;

        if (verts[i].pos.y < p0.y)
            p0.y = verts[i].pos.y;
        else if (verts[i].pos.y > p1.y)
            p1.y = verts[i].pos.y;

        if (verts[i].pos.z < p0.z)
            p0.z = verts[i].pos.z;
        else if (verts[i].pos.z > p1.z)
            p1.z = verts[i].pos.z;
    }

    // Prevent infinitesimally small bounds
    if (p0.x == p1.x)
    {
        p0.x -= BBOX_EPSILON;
        p1.x += BBOX_EPSILON;
    }
    if (p0.y == p1.y)
    {
        p0.y -= BBOX_EPSILON;
        p1.y += BBOX_EPSILON;
    }
    if (p0.z == p1.z)
    {
        p0.z -= BBOX_EPSILON;
        p1.z += BBOX_EPSILON;
    }

    return make_bbox(p0, p1);
}

static void bbox_init(struct bbox *box)
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
    // Get vertex shader attribute locations and set pointers (size/type/stride)
    //--------------------------------------------------------------------------
    if (box->prog->vert_pos >= 0)
    {
        glVertexAttribPointer(box->prog->vert_pos, 4, GL_FLOAT, GL_FALSE,
                              sizeof(struct vec4),
                              (GLvoid *)(0));
        glEnableVertexAttribArray(box->prog->vert_pos);
    }

    //--------------------------------------------------------------------------
    // Bind projection matrix
    //--------------------------------------------------------------------------
    //struct mat4 *proj_matrix = make_mat4_perspective(SCREEN_W, SCREEN_H,
    //                                                 Z_NEAR, Z_FAR, Z_FOV_DEG);
    //
    //glUniformMatrix4fv(box->program->u_proj, 1, GL_TRUE, proj_matrix->a);
    //
    //free_mat4(proj_matrix);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void bbox_render(const struct bbox *box, const struct mat4 *model_matrix)
{
    bbox_render_color(box, model_matrix, box->color);
}

void bbox_render_color(const struct bbox *box, const struct mat4 *model_matrix,
                       const struct col4 color)
{
    //--------------------------------------------------------------------------
    // Draw
    //--------------------------------------------------------------------------
    glUseProgram(box->prog->prog_id);

    glUniformMatrix4fv(box->prog->u_view, 1, GL_TRUE, view_matrix.a);

    // Model transform
    //model_matrix = make_mat4_ident();
    //mat4_scale(model_matrix, (struct vec4) { 10.0f, 10.0f, 10.0f });
    //mat4_translate(model_matrix, (struct vec4) { 0.0f, 0.0f, 0.0f });
    //mat4_roty(model_matrix, 30.0f);
    glUniformMatrix4fv(box->prog->u_model, 1, GL_TRUE, model_matrix->a);
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
    glUniform4f(box->prog->u_color, color.r, color.g, color.b, color.a);

    // Draw
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(box->vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, view_polygon_mode);

    glUseProgram(0);
    //glBindTexture(tex_default->target, 0);

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool bbox_intersects(const struct bbox *a, const struct bbox *b)
{
    if (a->vertices[7].x < b->vertices[0].x) return false;
    if (b->vertices[7].x < a->vertices[0].x) return false;

    if (a->vertices[7].y < b->vertices[0].y) return false;
    if (b->vertices[7].y < a->vertices[0].y) return false;

    if (a->vertices[7].z < b->vertices[0].z) return false;
    if (b->vertices[7].z < a->vertices[0].z) return false;

    return true;
}