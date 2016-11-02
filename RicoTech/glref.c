#include "glref.h"
#include "const.h"
#include "geom.h"
#include "util.h"
#include "shader.h"
#include "program.h"
#include "rico_texture.h"
#include "bbox.h"
#include "camera.h"
#include "rico_mesh.h"
#include "rico_obj.h"
#include "stb_image.h"
#include "font.h"

#include <GL/gl3w.h>
#include <SDL/SDL_assert.h>
#include <stdio.h>
#include <malloc.h>

//TODO: Implement better camera with position + lookat. Is that necessary?
//      Maybe it's easy to derive lookat when I need it? Probably not..
//struct vec4 camera_right = {  }

//static GLuint vao;
//static GLuint vbos[2];

static uint32 selected_handle = 0;

static struct program_default *prog_default;
static struct program_bbox *prog_bbox;

static struct rico_texture *tex_default;
static struct rico_texture *tex_hello1;
static struct rico_texture *tex_grass;

static struct rico_obj *obj_ground;
static struct rico_obj *obj_hello;
static struct rico_obj *obj_ruler;
static struct rico_obj *obj_wall1;
static struct rico_obj *obj_wall2;
static struct rico_obj *obj_wall3;
static struct rico_obj *obj_wall4;
static struct rico_obj *obj_wall5;
static struct rico_obj *arr_objects[50] = { 0 };
static int idx_arr_objects = 0;

static struct bbox axis_bbox;

static struct mat4 x_axis_transform;
static struct mat4 y_axis_transform;
static struct mat4 z_axis_transform;

void init_glref(struct rico_mesh **meshes, int mesh_count)
{
    // TODO: Create resource loaders to handle:
    //       fonts, shaders, textures, meshes, etc.

    //--------------------------------------------------------------------------
    // Initialize fonts
    //--------------------------------------------------------------------------
    struct font *font = make_font("courier_new.bff");

    //--------------------------------------------------------------------------
    // Initialize camera
    //--------------------------------------------------------------------------
    //Note: Player's eyes are at 1.7 meters
    view_camera.scale = (struct vec4) { 1.0f, 1.0f, 1.0f };
    view_camera.rot   = (struct vec4) { 0.0f, 0.0f, 0.0f };
    view_camera.trans = (struct vec4) { 0.0f,-1.7f,-4.0f };

    // Set projection matrix uniform
    mat4_perspective(&proj_matrix, SCREEN_W, SCREEN_H, Z_NEAR, Z_FAR,
                     Z_FOV_DEG);

    //--------------------------------------------------------------------------
    // Create shader program
    //--------------------------------------------------------------------------
    prog_default = make_program_default();
    if (!prog_default) return;

    prog_bbox = make_program_bbox();
    if (!prog_bbox) return;

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
    tex_grass = make_texture(GL_TEXTURE_2D, "grass.tga");
    tex_hello1 = make_texture(GL_TEXTURE_2D, "hello1.tga");
    tex_default = make_texture(GL_TEXTURE_2D, "basic.tga");

    //--------------------------------------------------------------------------
    // Create meshes
    //--------------------------------------------------------------------------
    const struct rico_mesh *mesh_grass =
        make_mesh("grass", prog_default, tex_grass, vertices, VERT_COUNT,
                  elements, ELEMENT_COUNT, GL_STATIC_DRAW);

    const struct rico_mesh *mesh_hello =
        make_mesh("hello", prog_default, tex_hello1, vertices, VERT_COUNT,
                  elements, ELEMENT_COUNT, GL_STATIC_DRAW);

    const struct rico_mesh *mesh_default =
        make_mesh("default", prog_default, tex_default, vertices, VERT_COUNT,
                  elements, ELEMENT_COUNT, GL_STATIC_DRAW);

    //--------------------------------------------------------------------------
    // Create world objects
    //--------------------------------------------------------------------------

    // Ground
    obj_ground = rico_obj_create("Ground", mesh_grass, &mesh_grass->bbox);
    obj_ground->trans = (struct vec4) { 0.0f, 0.0f, 0.0f, 1.0f };
    obj_ground->rot.x = -90.0f;
    obj_ground->scale = (struct vec4) { 64.0f, 64.0f, 1.0f, 1.0f };

    // Hello
    obj_hello = rico_obj_create("Hello", mesh_hello, &mesh_hello->bbox);
    obj_hello->trans = (struct vec4) { 0.0f, 2.0f, -4.0f };
    obj_hello->rot.y = 40.0f;
    obj_hello->scale = (struct vec4) { 1.0f, 2.0f, 1.0f };

    // Ruler
    obj_ruler = rico_obj_create("Ruler", mesh_default, &mesh_default->bbox);
    obj_ruler->trans = (struct vec4) { 0.0f, 1.0f, -3.0f };
    obj_ruler->scale = (struct vec4) { 1.0f, 1.0f, 1.0f, 1.0f };

    // Walls are all the same size for now
    struct vec4 wall_scale = (struct vec4) { 8.0f, 2.5f, 1.0f };

    // Wall front
    obj_wall1 = rico_obj_create("wall1", mesh_hello, &mesh_hello->bbox);
    obj_wall1->trans = (struct vec4) { 0.0f, 2.5f, -8.0f };
    obj_wall1->scale = wall_scale;

    // Wall left
    obj_wall2 = rico_obj_create("wall2", mesh_hello, &mesh_hello->bbox);
    obj_wall2->trans = (struct vec4) { -8.0f, 2.5f, 0.0f };
    obj_wall2->rot.y = 0.0f;
    obj_wall2->scale = wall_scale;

    // Wall back
    obj_wall3 = rico_obj_create("wall3", mesh_hello, &mesh_hello->bbox);
    obj_wall3->trans = (struct vec4) { 0.0f, 2.5f, 8.0f };
    obj_wall3->rot.y = 180.0f;
    obj_wall3->scale = wall_scale;

    // Wall right
    obj_wall4 = rico_obj_create("wall4", mesh_hello, &mesh_hello->bbox);
    obj_wall4->trans = (struct vec4) { 8.0f, 2.5f, 0.0f };
    obj_wall4->rot.y = -90.0f;
    obj_wall4->scale = wall_scale;

    // Wall five
    obj_wall5 = rico_obj_create("wall5", mesh_hello, &mesh_hello->bbox);
    obj_wall5->trans = (struct vec4) { 4.0f, 2.5f, 0.0f };
    obj_wall5->rot.y = -90.0f;
    obj_wall5->scale = wall_scale;

    {
        int i;
        for (i = 0; i < mesh_count; i++)
        {
            arr_objects[i] = rico_obj_create(meshes[i]->uid.name, meshes[i],
                                             &meshes[i]->bbox);
            arr_objects[i]->trans = (struct vec4) { 0.0f, 0.01f, 0.0f };
            arr_objects[i]->scale = VEC4_UNIT;
            
            // HACK: I want the walls to be taller for now
            if (i == 0) {
                arr_objects[i]->scale.x = 2.0f;
                arr_objects[i]->scale.z = 2.0f;
            }
            else {
                arr_objects[i]->scale.x = 2.0f;
                arr_objects[i]->scale.y = 2.0f;
                arr_objects[i]->scale.z = 2.0f;
            }
        }
        idx_arr_objects = i;
    }

    //--------------------------------------------------------------------------
    // Create axis label bboxes
    //--------------------------------------------------------------------------
    bbox_init(
        &axis_bbox,
        (struct vec4) { -0.5f, -0.5f, -0.5f, 1.0f },
        (struct vec4) {  0.5f,  0.5f,  0.5f, 1.0f },
        COLOR_WHITE
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

void select_obj(uint32 handle)
{
    // Deselect current object
    if (selected_handle > 0)
    {
        struct bbox *bbox = &rico_obj_fetch(selected_handle)->bbox;
        bbox->color = COLOR_GRAY_HIGHLIGHT;
        bbox->wireframe = true;
    }

    // Select requested object
    if (handle > 0)
    {
        struct bbox *bbox = &rico_obj_fetch(handle)->bbox;
        bbox->color = COLOR_RED_HIGHLIGHT;
        bbox->wireframe = false;
    }
    selected_handle = handle;
}
void select_next_obj()
{
    select_obj(rico_obj_next(selected_handle));
}
void select_prev_obj()
{
    select_obj(rico_obj_prev(selected_handle));
}

void translate_selected(struct vec4 offset)
{
    if (vec_equals(offset, VEC4_ZERO))
    {
        if (camera_lock)
            view_camera.trans = vec_add(view_camera.trans,
                                        rico_obj_fetch(selected_handle)->trans);

        rico_obj_fetch(selected_handle)->trans = VEC4_ZERO;
    }
    else
    {
        struct vec4 *trans = &rico_obj_fetch(selected_handle)->trans;
        *trans = vec_add(*trans, offset);

        if (camera_lock)
            view_camera.trans = vec_sub(view_camera.trans, offset);
    }
}
void rotate_selected(struct vec4 offset)
{
    if (vec_equals(offset, VEC4_ZERO))
    {
        rico_obj_fetch(selected_handle)->rot = VEC4_ZERO;
    }
    else
    {
        struct vec4 *rot = &rico_obj_fetch(selected_handle)->rot;
        *rot = vec_add(*rot, offset);
    }
}
void duplicate_selected()
{
    struct rico_obj *selected = rico_obj_fetch(selected_handle);
    
    int i = idx_arr_objects;

    char name[20];
    sprintf(name, "Duplicate %d", i);

    arr_objects[i] = rico_obj_create(name, selected->mesh,
                                     &selected->mesh->bbox);
    if (arr_objects[i])
    {
        arr_objects[i]->trans = selected->trans;
        arr_objects[i]->rot = selected->rot;
        arr_objects[i]->scale = selected->scale;
        idx_arr_objects++;
    }
}

//TODO: Put this somewhere reasonable
static struct col4 ambient = { 0.7f, 0.6f, 0.4f, 1.0f };
static GLfloat ambient_sign = -1.0f;

void update_glref(GLfloat dt, bool ambient_light)
{
    //static const struct col4 delta_ambient = { 0.01f, 0.01f, 0.01f, 0.0f };
    //
    //ambient.r += delta_ambient.r * dt * ambient_sign;
    //ambient.g += delta_ambient.g * dt * ambient_sign;
    //ambient.b += delta_ambient.b * dt * ambient_sign;
    //
    //GLfloat ambient_sum = ambient.r + ambient.g + ambient.b;
    //if (ambient_sum <= 0.1f || ambient_sum >= 2.7f)
    //    ambient_sign *= -1.0f;

    //--------------------------------------------------------------------------
    // Update uniforms
    //--------------------------------------------------------------------------
    glUseProgram(prog_default->prog_id);

    // Set uniforms
    glUniform1f(prog_default->u_time, dt);
    glUniformMatrix4fv(prog_default->u_view, 1, GL_TRUE, view_matrix.a);
    glUniformMatrix4fv(prog_default->u_proj, 1, GL_TRUE, proj_matrix.a);

    if (ambient_light)
        glUniform4fv(prog_default->u_ambient, 1, (const GLfloat *)&ambient);
    else
        glUniform4fv(prog_default->u_ambient, 1, (const GLfloat *)&VEC4_UNIT);

    glUseProgram(0);

    //--------------------------------------------------------------------------
    glUseProgram(prog_bbox->prog_id);

    // Set uniforms
    glUniformMatrix4fv(prog_bbox->u_view, 1, GL_TRUE, view_matrix.a);
    glUniformMatrix4fv(prog_bbox->u_proj, 1, GL_TRUE, proj_matrix.a);

    if (ambient_light)
        glUniform4fv(prog_default->u_ambient, 1, (const GLfloat *)&ambient);
    else
        glUniform4fv(prog_default->u_ambient, 1, (const GLfloat *)&VEC4_UNIT);

    glUseProgram(0);
}

void render_glref()
{
    //--------------------------------------------------------------------------
    // Render objects
    //--------------------------------------------------------------------------
    rico_obj_render(obj_ground);
    rico_obj_render(obj_hello);
    rico_obj_render(obj_ruler);
    rico_obj_render(obj_wall1);
    rico_obj_render(obj_wall2);
    rico_obj_render(obj_wall3);
    rico_obj_render(obj_wall4);

    for (int i = 0; arr_objects[i] != NULL; i++)
    {
        rico_obj_render(arr_objects[i]);
    }

    //--------------------------------------------------------------------------
    // Axes labels (bboxes)
    //--------------------------------------------------------------------------
    bbox_render_color(&axis_bbox, &x_axis_transform, COLOR_RED);
    bbox_render_color(&axis_bbox, &y_axis_transform, COLOR_GREEN);
    bbox_render_color(&axis_bbox, &z_axis_transform, COLOR_BLUE);
}
void free_glref()
{
    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    //TODO: Free all game objects
    free_rico_obj(&obj_ground);

    //TODO: Free all meshes
    
    //TODO: Free all textures
    free_texture(&tex_grass);
    free_texture(&tex_hello1);
    free_texture(&tex_default);

    //TODO: Free all programs
    free_program_default(&prog_default);
}