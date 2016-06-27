#include "glref.h"
#include "const.h"
#include "geom.h"
#include "util.h"
#include "shader.h"
#include "program.h"
#include "texture.h"
#include "stb_image.h"

#include <GL/gl3w.h>
#include <SDL/SDL_assert.h>
#include <stdio.h>
#include <malloc.h>

//TODO: Implement better camera with position + lookat. Is that necessary?
//      Maybe it's easy to derive lookat when I need it? Probably not..
//struct vec4 camera_right = {  }

static GLuint vao;
static GLuint vbos[2];

struct camera view_camera;
static struct program_default *prog_default;
static struct texture *tex_default;
static struct texture *tex_hello1;
static struct texture *tex_grass;

static struct bbox *obj_bbox;

enum { VBO_VERTEX, VBO_ELEMENT };

void init_glref()
{
    //--------------------------------------------------------------------------
    // Initialize camera
    //--------------------------------------------------------------------------
    
    //Note: Player's eyes are at 1.7 meters
    view_camera.scale = (struct vec4) { 1.0f, 1.0f, 1.0f };
    view_camera.rot   = (struct vec4) { 0.0f, 0.0f, 0.0f };
    view_camera.trans = (struct vec4) { 0.0f,-1.7f, 0.0f };

    //--------------------------------------------------------------------------
    // Create shaders
    //--------------------------------------------------------------------------
    prog_default = make_program_default();
    if (!prog_default) return;

    //--------------------------------------------------------------------------
    // Generate and bind VAOs
    //--------------------------------------------------------------------------
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //--------------------------------------------------------------------------
    // Generate buffer objects
    //--------------------------------------------------------------------------
    glGenBuffers(2, vbos);

    //--------------------------------------------------------------------------
    // Bind vertex data to buffer
    //--------------------------------------------------------------------------

    // Bind vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBO_VERTEX]);

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
    struct vertex vertices[VERT_COUNT] = {
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Alt: Update just the last vertex in the buffer

    //struct vertex new_subvertex;
    //glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) - sizeof(new_subvertex),
    //                    sizeof(new_subvertex), &new_subvertex);

    //--------------------------------------------------------------------------
    // Bind element data to buffer
    //--------------------------------------------------------------------------

    // Bind element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[VBO_ELEMENT]);

    GLuint elements[6] = { 0, 1, 3, 1, 2, 3 };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
                 GL_STATIC_DRAW);

    // Alt: Update just the last element index

    //GLuint new_subelement;
    //glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
    //                sizeof(elements) - sizeof(new_subelement),
    //                sizeof(new_subelement), &new_subelement);

    //--------------------------------------------------------------------------
    glUseProgram(prog_default->program);

    //--------------------------------------------------------------------------
    // Get vertex shader attribute locations and set pointers (size/type/stride)
    //--------------------------------------------------------------------------

    if (prog_default->vert_pos >= 0)
    {
        glVertexAttribPointer(prog_default->vert_pos, 4, GL_FLOAT, GL_FALSE,
                                10 * sizeof(GL_FLOAT),
                                (GLvoid *)(0));
        glEnableVertexAttribArray(prog_default->vert_pos);
    }

    if (prog_default->vert_col >= 0)
    {
        glVertexAttribPointer(prog_default->vert_col, 3, GL_FLOAT, GL_FALSE,
                                10 * sizeof(GL_FLOAT),
                                (GLvoid *)(4 * sizeof(GL_FLOAT)));
        glEnableVertexAttribArray(prog_default->vert_col);
    }

    if (prog_default->vert_uv >= 0)
    {
        glVertexAttribPointer(prog_default->vert_uv, 2, GL_FLOAT, GL_FALSE,
                                10 * sizeof(GL_FLOAT),
                                (GLvoid *)(8 * sizeof(GL_FLOAT)));
        glEnableVertexAttribArray(prog_default->vert_uv);
    }

    //--------------------------------------------------------------------------
    // Bind projection matrix
    //--------------------------------------------------------------------------
    mat5 proj_matrix = make_mat5_perspective(SCREEN_W, SCREEN_H,
                                                Z_NEAR, Z_FAR, Z_FOV_DEG);

    glUniformMatrix4fv(prog_default->u_proj, 1, GL_FALSE, proj_matrix);

    free_mat5(&proj_matrix);

    glUseProgram(0);
    //--------------------------------------------------------------------------

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //--------------------------------------------------------------------------
    // Create textures
    //--------------------------------------------------------------------------
    tex_default = make_texture(GL_TEXTURE_2D, "basic.tga");
    tex_hello1 = make_texture(GL_TEXTURE_2D, "hello1.tga");
    tex_grass = make_texture(GL_TEXTURE_2D, "grass.tga");

    //--------------------------------------------------------------------------
    // Create bounding box
    //--------------------------------------------------------------------------
    
    //TODO: Apply model transformations to bounding box
    obj_bbox = make_bbox_mesh(vertices, VERT_COUNT);
    free_bbox(&obj_bbox);
}

void update_glref(GLfloat dt)
{
    //--------------------------------------------------------------------------
    // Update uniforms
    //--------------------------------------------------------------------------

    /*************************************************************************
    | Row Major
    | model = trans * rot * scale
    | view = trans * rot * scale
    | gl_Position = proj * view * model * vec
    |
    **************************************************************************
    | Column Major
    | model = scale * rot * trans
    | view = scale * rot * trans
    | gl_Position = vec * model * view * proj
    |
    *************************************************************************/

    glUseProgram(prog_default->program);

    //Delta time
    glUniform1f(prog_default->u_time, dt);

    //View transform
    mat5 view_matrix = make_mat5_ident();
    mat5_scale(view_matrix, view_camera.scale);
    mat5_rotx(view_matrix, view_camera.rot.x);
    mat5_roty(view_matrix, view_camera.rot.y);
    mat5_rotz(view_matrix, view_camera.rot.z);
    mat5_translate(view_matrix, view_camera.trans);

    struct vec4 bleh = view_camera.trans;
    bleh.x = 5;

    glUniformMatrix4fv(prog_default->u_view, 1, GL_FALSE, view_matrix);

    glUseProgram(0);
}

void render_glref()
{
    //--------------------------------------------------------------------------
    // Draw
    //--------------------------------------------------------------------------

    mat5 model_matrix;
    struct tex2 uv_scale;

    //TODO: Don't have multiple textures / model matrices for single mesh
    //      Let the mesh render itself, set uniforms in mesh_init()

    //--------------------------------------------------------------------------
    // Ground object
    //--------------------------------------------------------------------------

    glUseProgram(prog_default->program);

        // Model transform
        model_matrix = make_mat5_ident();
        //mat5_translate(model_matrix, (struct vec4) { 0.0f, 0.0f, 0.0f });
        mat5_rotx(model_matrix, -90.0f);
        mat5_scale(model_matrix, (struct vec4) { 64.0f, 64.0f, 1.0f });
        glUniformMatrix4fv(prog_default->u_model, 1, GL_FALSE, model_matrix);
    
        // Model texture
        // Note: We don't have to do this every time as long as we make sure
        //       the correct textures are bound before each draw to the texture
        //       index assumed when the program was initialized.
        //glUniform1i(prog_default->u_tex, 0);

        // UV-coord scale
        uv_scale = (struct tex2) { 128.0f, 128.0f };
        glUniform2f(prog_default->u_scale_uv, uv_scale.u, uv_scale.v);

        // Bind texture(s)
        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(tex_grass->target, tex_grass->texture);

        // Draw
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    glUseProgram(0);
    glBindTexture(tex_grass->target, 0);

    //--------------------------------------------------------------------------
    // Ruler object
    //--------------------------------------------------------------------------

    glUseProgram(prog_default->program);

        // Model transform
        model_matrix = make_mat5_ident();
        mat5_translate(model_matrix, (struct vec4) { 0.0f, 1.0f, -3.0f });
        //mat5_scale(model_matrix, (struct vec4) { 1.0f, 1.0f, 1.0f });
        glUniformMatrix4fv(prog_default->u_model, 1, GL_FALSE, model_matrix);
    
        // Model texture
        // Note: We don't have to do this every time as long as we make sure
        //       the correct textures are bound before each draw to the texture
        //       index assumed when the program was initialized.
        //glUniform1i(prog_default->u_tex, 0);

        // UV-coord scale
        uv_scale = (struct tex2) { 2.0f, 2.0f };
        glUniform2f(prog_default->u_scale_uv, uv_scale.u, uv_scale.v);

        // Bind texture(s)
        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(tex_default->target, tex_default->texture);

        // Draw
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    glUseProgram(0);
    glBindTexture(tex_default->target, 0);

    //--------------------------------------------------------------------------
    // Hello object
    //--------------------------------------------------------------------------

    glUseProgram(prog_default->program);

        // Model transform
        model_matrix = make_mat5_ident();
        mat5_scale(model_matrix, (struct vec4) { 1.0f, 2.0f, 1.0f });
        mat5_translate(model_matrix, (struct vec4) { 0.0f, 1.0f, -4.0f });
        mat5_roty(model_matrix, 30.0f);
        glUniformMatrix4fv(prog_default->u_model, 1, GL_FALSE, model_matrix);

        // Model texture
        // Note: We don't have to do this every time as long as we make sure
        //       the correct textures are bound before each draw to the texture
        //       index assumed when the program was initialized.
        //glUniform1i(prog_default->u_tex, 0);

        // UV-coord scale
        uv_scale = (struct tex2) { 1.0f, 2.0f };
        glUniform2f(prog_default->u_scale_uv, uv_scale.u, uv_scale.v);

        // Bind texture(s)
        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(tex_hello1->target, tex_hello1->texture);

        // Draw
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    glUseProgram(0);
    glBindTexture(tex_default->target, 0);

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void free_glref()
{
    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    glDeleteBuffers(2, vbos);
    glDeleteVertexArrays(1, &vao);

    free_texture(&tex_default);
    free_program_default(&prog_default);
}