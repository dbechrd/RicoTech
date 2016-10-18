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
static struct rico_obj obj_ruler;
static struct rico_obj obj_wall1;
static struct rico_obj obj_wall2;
static struct rico_obj obj_wall3;
static struct rico_obj obj_wall4;
static struct rico_obj obj_hello;

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
    // Create  meshes
    //--------------------------------------------------------------------------
    
    // Ground
    obj_ground.trans = (struct vec4) { 0.0f, 0.0f, 0.0f, 1.0f };
    obj_ground.rot.x = -90.0f;
    obj_ground.scale = (struct vec4) { 64.0f, 64.0f, 1.0f, 1.0f };
    obj_ground.mesh = make_mesh(prog_default, tex_grass, vertices, VERT_COUNT,
                                elements, ELEMENT_COUNT, GL_STATIC_DRAW);

    struct rico_mesh *mesh_rect_default =
        make_mesh(prog_default, tex_default, vertices, VERT_COUNT, elements,
                  ELEMENT_COUNT, GL_STATIC_DRAW);

    // Ruler
    obj_ruler.trans = (struct vec4) { 0.0f, 1.0f, -3.0f };
    obj_ruler.scale = (struct vec4) { 1.0f, 1.0f, 1.0f, 1.0f };
    obj_ruler.mesh = mesh_rect_default;

    // Walls are all the same size for now
    struct vec4 wall_scale = (struct vec4) { 8.0f, 2.5f, 1.0f };

    // Wall front
    obj_wall1.trans = (struct vec4) { 0.0f, 2.5f, -8.0f };
    obj_wall1.scale = wall_scale;
    obj_wall1.mesh = mesh_rect_default;

    // Wall left
    obj_wall2.trans = (struct vec4) { -8.0f, 2.5f, 0.0f };
    obj_wall2.rot.y = 0.0f;
    obj_wall2.scale = wall_scale;
    obj_wall2.mesh = mesh_rect_default;

    // Wall back
    obj_wall3.trans = (struct vec4) { 0.0f, 2.5f, 8.0f };
    obj_wall3.rot.y = 180.0f;
    obj_wall3.scale = wall_scale;
    obj_wall3.mesh = mesh_rect_default;

    // Wall right
    obj_wall4.trans = (struct vec4) { 8.0f, 2.5f, 0.0f };
    obj_wall4.rot.y = -90.0f;
    obj_wall4.scale = wall_scale;
    obj_wall4.mesh = mesh_rect_default;

    // Hello
    obj_hello.trans = (struct vec4) { 0.0f, 2.0f, -4.0f };
    obj_hello.rot.y = 30.0f;
    obj_hello.scale = (struct vec4) { 1.0f, 2.0f, 1.0f };
    obj_hello.mesh = make_mesh(prog_default, tex_hello1, vertices, VERT_COUNT,
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
    // Render objects
    //--------------------------------------------------------------------------
    rico_obj_render(&obj_ground);
    rico_obj_render(&obj_ruler);
    
    rico_obj_render(&obj_wall1);
    rico_obj_render(&obj_wall2);
    rico_obj_render(&obj_wall3);
    rico_obj_render(&obj_wall4);

    rico_obj_render(&obj_hello);

    //--------------------------------------------------------------------------
    // Axes labels (bboxes)
    //--------------------------------------------------------------------------
    bbox_render_color(axis_bbox, &x_axis_transform, COLOR_RED);
    bbox_render_color(axis_bbox, &y_axis_transform, COLOR_GREEN);
    bbox_render_color(axis_bbox, &z_axis_transform, COLOR_BLUE);
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