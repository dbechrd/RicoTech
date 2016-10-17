#include "glref.h"
#include "const.h"
#include "geom.h"
#include "util.h"
#include "shader.h"
#include "program.h"
#include "texture.h"
#include "bbox.h"
#include "camera.h"
#include "mesh.h"
#include "rico_obj.h"
#include "stb_image.h"

#include <GL/gl3w.h>
#include <SDL/SDL_assert.h>
#include <stdio.h>
#include <malloc.h>

//TODO: Implement better camera with position + lookat. Is that necessary?
//      Maybe it's easy to derive lookat when I need it? Probably not..
//struct vec4 camera_right = {  }

//static GLuint vao;
//static GLuint vbos[2];

static struct program_default *prog_default;
static struct program_bbox *prog_bbox;

static struct rico_texture *tex_default;
static struct rico_texture *tex_hello1;
static struct rico_texture *tex_grass;

static struct rico_obj obj_ground;

static const struct bbox *axis_bbox;

static struct mat4 x_axis_transform;
static struct mat4 y_axis_transform;
static struct mat4 z_axis_transform;

void init_glref()
{
    //--------------------------------------------------------------------------
    // Initialize camera
    //--------------------------------------------------------------------------
    
    //Note: Player's eyes are at 1.7 meters
    view_camera.scale = (struct vec4) { 1.0f, 1.0f, 1.0f };
    view_camera.rot   = (struct vec4) { 0.0f, 0.0f, 0.0f };
    view_camera.trans = (struct vec4) { 0.0f,-1.7f,-4.0f };

    //--------------------------------------------------------------------------
    // Create shader program
    //--------------------------------------------------------------------------
    prog_default = make_program_default();
    if (!prog_default) return;

    prog_bbox = make_program_bbox();
    if (!prog_bbox) return;

    // Set projection matrix uniform
    mat4_perspective(&proj_matrix, SCREEN_W, SCREEN_H, Z_NEAR, Z_FAR,
                     Z_FOV_DEG);

    /*************************************************************************
    | Frequency of access:
    | 
    | STREAM  Data store contents modified once and used at most a few times.
    | STATIC  Data store contents modified once and used many times.
    | DYNAMIC Data store contents modified repeatedly and used many times.
    |
    **************************************************************************
    | Nature of access:
    | 
    | DRAW    The data store contents are modified by the application, and used
    |         as the source for GL drawing and image specification commands.
    | READ    The data store contents are modified by reading data from the GL,
    |         and used to return that data when queried by the application.
    | COPY    DRAW & READ
    |
    *************************************************************************/
    #define VERT_COUNT 4
    const struct mesh_vertex vertices[VERT_COUNT] = {
        {
            { -1.0f, -1.0f, 0.0f, 1.0f }, //Position
            { 1.0f, 1.0f, 1.0f, 1.0f },   //Color
            { 0.0f, 0.0f }                //UV-coords
        },
        {
            { 1.0f, -1.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        {
            { 1.0f, 1.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f }
        },
        {
            { -1.0f, 1.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f }
        }
    };
    #define ELEMENT_COUNT 6
    const GLuint elements[ELEMENT_COUNT] = { 0, 1, 3, 1, 2, 3 };

    //--------------------------------------------------------------------------
    // Create textures
    //--------------------------------------------------------------------------
    tex_default = make_texture(GL_TEXTURE_2D, "basic.tga");
    tex_hello1 = make_texture(GL_TEXTURE_2D, "hello1.tga");
    tex_grass = make_texture(GL_TEXTURE_2D, "grass.tga");

    //--------------------------------------------------------------------------
    // Create mesh
    //--------------------------------------------------------------------------
    obj_ground.trans = (struct vec4) { 0.0f, 0.0f, 0.0f, 1.0f };
    obj_ground.rot.x = -90.0f;
    obj_ground.scale = (struct vec4) { 64.0f, 64.0f, 1.0f, 1.0f };

    obj_ground.mesh = make_mesh(prog_default, tex_grass, vertices, VERT_COUNT,
                                elements, ELEMENT_COUNT, GL_STATIC_DRAW);

    //--------------------------------------------------------------------------
    // Create axis label bboxes
    //--------------------------------------------------------------------------
    axis_bbox = make_bbox(
        (struct vec4) { -0.5f, -0.5f, -0.5f, 1.0f },
        (struct vec4) {  0.5f,  0.5f,  0.5f, 1.0f }
    );

    // X-axis label
    mat4_ident(&x_axis_transform);
    mat4_scale(&x_axis_transform, (struct vec4) { 1.0f, 0.01f, 0.01f });
    mat4_translate(&x_axis_transform, (struct vec4) { 0.5f, 0.0f, 0.0f });

    // Y-axis label
    mat4_ident(&y_axis_transform);
    mat4_scale(&y_axis_transform, (struct vec4) { 0.01f, 1.0f, 0.01f });
    mat4_translate(&y_axis_transform, (struct vec4) { 0.0f, 0.5f, 0.0f });

    // Z-axis label
    mat4_ident(&z_axis_transform);
    mat4_scale(&z_axis_transform, (struct vec4) { 0.01f, 0.01f, 1.0f });
    mat4_translate(&z_axis_transform, (struct vec4) { 0.0f, 0.0f, 0.5f });
}

void update_glref(GLfloat dt)
{
    //--------------------------------------------------------------------------
    // Update uniforms
    //--------------------------------------------------------------------------
    glUseProgram(prog_default->prog_id);

    // Set uniforms
    glUniform1f(prog_default->u_time, dt);
    glUniformMatrix4fv(prog_default->u_view, 1, GL_TRUE, view_matrix.a);
    glUniformMatrix4fv(prog_default->u_proj, 1, GL_TRUE, proj_matrix.a);

    glUseProgram(0);

    //--------------------------------------------------------------------------
    glUseProgram(prog_bbox->prog_id);

    // Set uniforms
    glUniformMatrix4fv(prog_bbox->u_view, 1, GL_TRUE, view_matrix.a);
    glUniformMatrix4fv(prog_bbox->u_proj, 1, GL_TRUE, proj_matrix.a);

    glUseProgram(0);
}

void render_glref()
{
    //--------------------------------------------------------------------------
    // Draw
    //--------------------------------------------------------------------------

    //struct mat4 model_matrix;
    //struct tex2 uv_scale;

    //TODO: Don't have multiple textures / model matrices for single mesh
    //      Let the mesh render itself, set uniforms in mesh_init()

    //--------------------------------------------------------------------------
    // Ground object
    //--------------------------------------------------------------------------
    rico_obj_render(&obj_ground);

    //--------------------------------------------------------------------------
    // Axes labels (bboxes)
    //--------------------------------------------------------------------------
    bbox_render_color(axis_bbox, &x_axis_transform, COLOR_RED);
    bbox_render_color(axis_bbox, &y_axis_transform, COLOR_GREEN);
    bbox_render_color(axis_bbox, &z_axis_transform, COLOR_BLUE);

    ////--------------------------------------------------------------------------
    //// Ruler object
    ////--------------------------------------------------------------------------
    //glUseProgram(prog_default->prog_id);

    //    // Model transform
    //    mat4_ident(&model_matrix);
    //    mat4_translate(&model_matrix, (struct vec4) { 0.0f, 1.0f, -3.0f });
    //    //mat4_scale(model_matrix, (struct vec4) { 1.0f, 1.0f, 1.0f });
    //    glUniformMatrix4fv(prog_default->u_model, 1, GL_TRUE, model_matrix.a);

    //    // Model texture
    //    // Note: We don't have to do this every time as long as we make sure
    //    //       the correct textures are bound before each draw to the texture
    //    //       index assumed when the program was initialized.
    //    //glUniform1i(prog_default->u_tex, 0);

    //    // UV-coord scale
    //    uv_scale = (struct tex2) { 1.0f, 1.0f };
    //    glUniform2f(prog_default->u_scale_uv, uv_scale.u, uv_scale.v);

    //    // Bind texture(s)
    //    //glActiveTexture(GL_TEXTURE0);
    //    glBindTexture(tex_default->target, tex_default->texture);

    //    // Draw
    //    glBindVertexArray(vao);
    //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    //    glBindVertexArray(0);

    //glUseProgram(0);

    //bbox_render(obj_bbox, &model_matrix);

    ////--------------------------------------------------------------------------
    //// Walls, because I can
    ////--------------------------------------------------------------------------

    //struct mat4 *model_matrix_wall1;
    //struct mat4 *model_matrix_wall2;
    //struct mat4 *model_matrix_wall3;
    //struct mat4 *model_matrix_wall4;

    //glUseProgram(prog_default->prog_id);
    //glBindVertexArray(vao);

    //// Bind texture
    //glBindTexture(tex_default->target, tex_default->texture);

    //// UV-coord scale
    //uv_scale = (struct tex2) { 8.0f, 2.5f };
    //glUniform2f(prog_default->u_scale_uv, uv_scale.u, uv_scale.v);

    ////--------------------------------------------------------------------------

    //// Model transform
    //model_matrix_wall1 = make_mat4_ident();
    //mat4_translate(model_matrix_wall1, (struct vec4) { 0.0f, 2.5f, -8.0f });
    //mat4_scale(model_matrix_wall1, (struct vec4) { 8.0f, 2.5f, 1.0f });
    //glUniformMatrix4fv(prog_default->u_model, 1, GL_TRUE,
    //                   model_matrix_wall1->a);

    //// Draw
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    ////--------------------------------------------------------------------------

    //// Model transform
    //model_matrix_wall2 = make_mat4_ident();
    //mat4_translate(model_matrix_wall2, (struct vec4) { -8.0f, 2.5f, 0.0f });
    //mat4_roty(model_matrix_wall2, 90.0f);
    //mat4_scale(model_matrix_wall2, (struct vec4) { 8.0f, 2.5f, 1.0f });
    //glUniformMatrix4fv(prog_default->u_model, 1, GL_TRUE,
    //                   model_matrix_wall2->a);

    //// Draw
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    ////--------------------------------------------------------------------------

    //// Model transform
    //model_matrix_wall3 = make_mat4_ident();
    //mat4_translate(model_matrix_wall3, (struct vec4) { 0.0f, 2.5f, 8.0f });
    //mat4_roty(model_matrix_wall3, 180.0f);
    //mat4_scale(model_matrix_wall3, (struct vec4) { 8.0f, 2.5f, 1.0f });
    //glUniformMatrix4fv(prog_default->u_model, 1, GL_TRUE,
    //                   model_matrix_wall3->a);

    //// Draw
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    ////--------------------------------------------------------------------------

    //// Model transform
    //model_matrix_wall4 = make_mat4_ident();
    //mat4_translate(model_matrix_wall4, (struct vec4) { 8.0f, 2.5f, 0.0f });
    //mat4_roty(model_matrix_wall4, -90.0f);
    //mat4_scale(model_matrix_wall4, (struct vec4) { 8.0f, 2.5f, 1.0f });
    //glUniformMatrix4fv(prog_default->u_model, 1, GL_TRUE,
    //                   model_matrix_wall4->a);

    //// Draw
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    ////--------------------------------------------------------------------------

    //glBindVertexArray(0);
    //glUseProgram(0);
    //glBindTexture(tex_default->target, 0);

    //bbox_render(obj_bbox, model_matrix_wall1);
    //bbox_render(obj_bbox, model_matrix_wall2);
    //bbox_render(obj_bbox, model_matrix_wall3);
    //bbox_render(obj_bbox, model_matrix_wall4);

    //free_mat4(model_matrix_wall1);
    //free_mat4(model_matrix_wall2);
    //free_mat4(model_matrix_wall3);
    //free_mat4(model_matrix_wall4);

    ////--------------------------------------------------------------------------
    //// Hello object
    ////--------------------------------------------------------------------------
    //glUseProgram(prog_default->prog_id);

    //    // Model transform
    //    mat4_ident(&model_matrix);
    //    mat4_scale(&model_matrix, (struct vec4) { 1.0f, 2.0f, 1.0f });
    //    mat4_translate(&model_matrix, (struct vec4) { 0.0f, 1.0f, -4.0f });
    //    mat4_roty(&model_matrix, 30.0f);
    //    glUniformMatrix4fv(prog_default->u_model, 1, GL_TRUE, model_matrix.a);

    //    // Model texture
    //    // Note: We don't have to do this every time as long as we make sure
    //    //       the correct textures are bound before each draw to the texture
    //    //       index assumed when the program was initialized.
    //    //glUniform1i(prog_default->u_tex, 0);

    //    // UV-coord scale
    //    uv_scale = (struct tex2) { 1.0f, 2.0f };
    //    glUniform2f(prog_default->u_scale_uv, uv_scale.u, uv_scale.v);

    //    // Bind texture(s)
    //    //glActiveTexture(GL_TEXTURE0);
    //    glBindTexture(tex_hello1->target, tex_hello1->texture);

    //    // Draw
    //    glBindVertexArray(vao);
    //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    //    glBindVertexArray(0);

    //glUseProgram(0);
    //glBindTexture(tex_default->target, 0);

    //bbox_render(obj_bbox, &model_matrix);

    ////--------------------------------------------------------------------------
    //// Clean up
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void free_glref()
{
    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    free_texture(&tex_default);
    free_texture(&tex_hello1);
    free_texture(&tex_grass);
    free_mesh(&obj_ground.mesh);
    free_program_default(&prog_default);
}