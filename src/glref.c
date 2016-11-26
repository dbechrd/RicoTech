#include "glref.h"
#include "const.h"
#include "geom.h"
#include "util.h"
#include "shader.h"
#include "program.h"
#include "bbox.h"
#include "camera.h"
#include "rico_texture.h"
#include "rico_mesh.h"
#include "rico_object.h"
#include "rico_font.h"
#include "rico_chunk.h"
#include "rico_pool.h"

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

static uint32 tex_grass;
static uint32 tex_rock;
static uint32 tex_hello;
static uint32 tex_font_test;

static uint32 mesh_default;
static uint32 mesh_font_test;

static struct bbox axis_bbox;

static struct mat4 x_axis_transform;
static struct mat4 y_axis_transform;
static struct mat4 z_axis_transform;

int init_glref()
{
    int err;

    //--------------------------------------------------------------------------
    // Initialize fonts
    //--------------------------------------------------------------------------
    // TODO: Add error handling to make_font()
    struct rico_font *font = make_font("font/courier_new.bff");

    //--------------------------------------------------------------------------
    // Initialize camera
    //--------------------------------------------------------------------------
    //Note: Player's eyes are at 1.7 meters
    view_camera.scale = VEC4_UNIT;
    view_camera.rot   = VEC4_ZERO;
    view_camera.trans = (struct vec4) { 0.0f, -1.7f, -4.0f, 0.0f };

    // Set projection matrix uniform
    mat4_perspective(&proj_matrix, SCREEN_W, SCREEN_H, Z_NEAR, Z_FAR,
                     Z_FOV_DEG);

    //--------------------------------------------------------------------------
    // Create shader program
    //--------------------------------------------------------------------------
    err = make_program_default(&prog_default);
    if (err) return err;

    err = make_program_bbox(&prog_bbox);
    if (err) return err;

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
    // Create meshes
    //--------------------------------------------------------------------------
    err = mesh_load("default", VERT_COUNT, vertices, ELEMENT_COUNT, elements,
                    GL_STATIC_DRAW, &mesh_default);
    if (err) return err;

    //--------------------------------------------------------------------------
    // Create textures
    //--------------------------------------------------------------------------
    texture_load_file("grass", GL_TEXTURE_2D, "texture/grass.tga", &tex_grass);
    texture_load_file("bricks", GL_TEXTURE_2D, "texture/clean_bricks.tga", &tex_rock);
    texture_load_file("hello", GL_TEXTURE_2D, "texture/hello.tga", &tex_hello);

    // Cleanup: Memory pool free/reuse test
    texture_free(tex_grass);
    texture_load_file("grass", GL_TEXTURE_2D, "texture/grass.tga", &tex_grass);

    //--------------------------------------------------------------------------
    // Create 3D strings
    //--------------------------------------------------------------------------
    // Font Test
    font_render(font, 0, 0, "This is a test.\nYay!", COLOR_RED, &mesh_font_test,
                &tex_font_test);

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
    mat4_scale(&x_axis_transform, (struct vec4) { 1.0f, 0.01f, 0.01f, 1.0f });
    mat4_translate(&x_axis_transform, (struct vec4) { 0.5f, 0.0f, 0.0f, 1.0f });

    // Y-axis label
    mat4_ident(&y_axis_transform);
    mat4_scale(&y_axis_transform, (struct vec4) { 0.01f, 1.0f, 0.01f, 1.0f });
    mat4_translate(&y_axis_transform, (struct vec4) { 0.0f, 0.5f, 0.0f, 1.0f });

    // Z-axis label
    mat4_ident(&z_axis_transform);
    mat4_scale(&z_axis_transform, (struct vec4) { 0.01f, 0.01f, 1.0f, 1.0f });
    mat4_translate(&z_axis_transform, (struct vec4) { 0.0f, 0.0f, 0.5f, 1.0f });

    return SUCCESS;
}

int init_manual_chunk(uint32 *meshes, uint32 mesh_count)
{
    int err;
    uint32 obj_fonttest;
    uint32 obj_ground;
    uint32 obj_hello;
    uint32 obj_ruler;
    uint32 obj_wall1;
    uint32 obj_wall2;
    uint32 obj_wall3;
    uint32 obj_wall4;
    uint32 obj_wall5;
    uint32 arr_objects[50] = { 0 };

    // Initialize object pool
    object_init(RICO_OBJECT_POOL_SIZE);

    //--------------------------------------------------------------------------
    // Create world objects
    //--------------------------------------------------------------------------
    object_create("Font Test", mesh_font_test, tex_font_test, NULL,
                  &obj_fonttest);
    object_trans(obj_fonttest, 0.0f, 2.0f, 0.0f);
    object_scale(obj_fonttest, 1.0f, 1.0f, 1.0f);

    // Ground
    object_create("Ground", mesh_default, tex_rock, NULL, &obj_ground);
    object_rot_x(obj_ground, -90.0f);
    object_scale(obj_ground, 64.0f, 64.0f, 1.0f);

    // Hello
    object_create("Hello", mesh_default, tex_hello, NULL, &obj_hello);
    object_trans(obj_hello, 0.0f, 2.0f, -4.0f);
    object_rot_y(obj_hello, 40.0f);
    object_scale(obj_hello, 1.0f, 2.0f, 1.0f);

    // Ruler
    object_create("Ruler", mesh_default, RICO_TEXTURE_DEFAULT, NULL, &obj_ruler);
    object_trans(obj_ruler, 0.0f, 1.0f, -3.0f);
    object_scale(obj_ruler, 1.0f, 1.0f, 1.0f);

    // Walls are all the same size for now
    struct vec4 wall_scale = (struct vec4) { 8.0f, 2.5f, 1.0f, 1.0f };

    // Wall front
    object_create("wall1", mesh_default, tex_hello, NULL, &obj_wall1);
    object_trans(obj_wall1, 0.0f, 2.5f, -8.0f);
    object_scale(obj_wall1, wall_scale.x, wall_scale.y, wall_scale.z);

    // Wall left
    object_create("wall2", mesh_default, tex_hello, NULL, &obj_wall2);
    object_trans(obj_wall2, -8.0f, 2.5f, 0.0f);
    object_scale(obj_wall2, wall_scale.x, wall_scale.y, wall_scale.z);

    // Wall back
    object_create("wall3", mesh_default, tex_hello, NULL, &obj_wall3);
    object_trans(obj_wall3, 0.0f, 2.5f, 8.0f);
    object_rot_y(obj_wall3, 180.0f);
    object_scale(obj_wall3, wall_scale.x, wall_scale.y, wall_scale.z);

    // Wall right
    object_create("wall4", mesh_default, tex_hello, NULL, &obj_wall4);
    object_trans(obj_wall4, 8.0f, 2.5f, 0.0f);
    object_rot_y(obj_wall4, -90.0f);
    object_scale(obj_wall4, wall_scale.x, wall_scale.y, wall_scale.z);

    // Wall five
    object_create("wall5", mesh_default, tex_hello, NULL, &obj_wall5);
    object_trans(obj_wall5, 4.0f, 2.5f, 0.0f);
    object_rot_y(obj_wall5, -90.0f);
    object_scale(obj_wall5, wall_scale.x, wall_scale.y, wall_scale.z);

    {
        for (uint32 i = 0; i < mesh_count; i++)
        {
            object_create(mesh_name(meshes[i]), meshes[i], RICO_TEXTURE_DEFAULT,
                          mesh_bbox(meshes[i]), &arr_objects[i]);

            // HACK: Don't z-fight ground plane
            object_trans(arr_objects[i], 0.0f, EPSILON, 0.0f);
        }
    }

    //--------------------------------------------------------------------------
    // Save manual chunk
    //--------------------------------------------------------------------------
    struct rico_pool *chunk_tex_pool = texture_pool_unsafe();
    struct rico_pool *chunk_mesh_pool = mesh_pool_unsafe();
    struct rico_pool *chunk_obj_pool = object_pool_get_unsafe();

    struct rico_chunk chunkA;

    err = chunk_init("my_first_chunk", chunk_tex_pool->count, chunk_mesh_pool->count,
                     chunk_obj_pool->count, chunk_tex_pool, chunk_mesh_pool,
                     chunk_obj_pool, &chunkA);
    if (err) return err;

    err = chunk_save("../res/chunks/chunky.bin", &chunkA);
    return err;
}

void select_obj(uint32 handle)
{
    // Deselect current object
    object_deselect(selected_handle);

    // Select requested object
    object_select(handle);

    selected_handle = handle;

    struct rico_object *obj = object_fetch(selected_handle);
    printf("[Obj %d][%d %s] Selected\n", selected_handle, obj->uid.uid,
           obj->uid.name);
}

void select_next_obj()
{
    select_obj(object_next(selected_handle));
}

void select_prev_obj()
{
    select_obj(object_prev(selected_handle));
}

void translate_selected(struct vec4 offset)
{
    struct rico_object *obj = object_fetch(selected_handle);

    if (vec_equals(offset, VEC4_ZERO))
    {
        if (camera_lock)
        {
            view_camera.trans = vec_add(view_camera.trans, obj->trans);
        }
        obj->trans = VEC4_ZERO;
    }
    else
    {
        if (camera_lock)
        {
            view_camera.trans = vec_sub(view_camera.trans, offset);
        }
        obj->trans = vec_add(obj->trans, offset);
    }
}

void rotate_selected(struct vec4 offset)
{
    if (vec_equals(offset, VEC4_ZERO))
    {
        object_fetch(selected_handle)->rot = VEC4_ZERO;
    }
    else
    {
        struct vec4 *rot = &object_fetch(selected_handle)->rot;
        *rot = vec_add(*rot, offset);
    }
}

void duplicate_selected()
{
    struct rico_object *selected = object_fetch(selected_handle);

    uint32 newObj;
    object_create("Duplicate", selected->mesh, selected->texture,
                  mesh_bbox(selected->mesh), &newObj);

    struct rico_object *new_obj = object_fetch(newObj);
    if (newObj)
    {
        new_obj->trans = selected->trans;
        new_obj->rot = selected->rot;
        new_obj->scale = selected->scale;
    }

    select_obj(newObj);
}

void delete_selected()
{
    uint32 handle = selected_handle;
    select_prev_obj();
    object_free(handle);
}

//TODO: Put this somewhere reasonable (e.g. lighting module)
static struct col4 ambient = { 0.7f, 0.6f, 0.4f, 1.0f };

void update_glref(GLfloat dt, bool ambient_light)
{
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
    object_render_all(prog_default);

    // object_render(obj_ground, prog_default);
    // object_render(obj_hello, prog_default);
    // object_render(obj_ruler, prog_default);
    // object_render(obj_wall1, prog_default);
    // object_render(obj_wall2, prog_default);
    // object_render(obj_wall3, prog_default);
    // object_render(obj_wall4, prog_default);

    // for (uint32 i = 0; i < idx_arr_objects; i++)
    // {
    //     object_render(arr_objects[i], prog_default);
    // }

    //--------------------------------------------------------------------------
    // Axes labels (bboxes)
    //--------------------------------------------------------------------------
    bbox_render_color(&axis_bbox, &x_axis_transform, COLOR_RED);
    bbox_render_color(&axis_bbox, &y_axis_transform, COLOR_GREEN);
    bbox_render_color(&axis_bbox, &z_axis_transform, COLOR_BLUE);

    // object_render(obj_fonttest, prog_default);
}
void free_glref()
{
    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    //TODO: Free all game objects
    object_free_all();

    //TODO: Free all meshes

    //TODO: Free all textures
    texture_free(tex_grass);
    texture_free(tex_rock);
    texture_free(tex_hello);

    //TODO: Free all programs
    free_program_default(&prog_default);
}