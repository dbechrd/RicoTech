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
//struct vec3 camera_right = {  }

//static GLuint vao;
//static GLuint vbos[2];

static u32 selected_handle = 0;

static struct program_default *prog_default;
static struct program_primitive *prog_primitive;

static u32 font;
static u32 tex_font_test;
static u32 mesh_font_test;
static u32 mesh_default;
static u32 tex_grass;
static u32 tex_rock;
static u32 tex_hello;
static u32 tex_yellow;

static struct bbox axis_bbox;

static struct mat4 x_axis_transform;
static struct mat4 y_axis_transform;
static struct mat4 z_axis_transform;

static u32 obj_debuginfostring;

int init_glref()
{
    enum rico_error err;

    //--------------------------------------------------------------------------
    // Initialize fonts
    //--------------------------------------------------------------------------
    // TODO: Add error handling to make_font()
    font_init("font/courier_new.bff", &font);

    //--------------------------------------------------------------------------
    // Create shader program
    //--------------------------------------------------------------------------
    err = make_program_default(&prog_default);
    if (err) return err;

    err = make_program_primitive(&prog_primitive);
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

    //--------------------------------------------------------------------------
    // Create textures
    //--------------------------------------------------------------------------
    err = texture_load_file("grass", GL_TEXTURE_2D, "texture/grass.tga",
                            &tex_grass);
    if (err) return err;
    err = texture_load_file("bricks", GL_TEXTURE_2D, "texture/clean_bricks.tga",
                            &tex_rock);
    if (err) return err;
    err = texture_load_file("hello", GL_TEXTURE_2D, "texture/hello.tga",
                            &tex_hello);
    if (err) return err;
    err = texture_load_file("yellow", GL_TEXTURE_2D, "texture/fake_yellow.tga",
                            &tex_yellow);
    if (err) return err;

    //--------------------------------------------------------------------------
    // Create 3D strings
    //--------------------------------------------------------------------------
    // Font Test
    err = font_render(font, 0, 0, COLOR_DARK_RED_HIGHLIGHT,
                      "This is a test.\nYay!", "Font test", &mesh_font_test,
                      &tex_font_test);
    if (err) return err;

    //--------------------------------------------------------------------------
    // Create axis label bboxes
    //--------------------------------------------------------------------------
    err = bbox_init(
        &axis_bbox,
        "Axis BBox",
        (struct vec3) { -0.5f, -0.5f, -0.5f },
        (struct vec3) {  0.5f,  0.5f,  0.5f },
        COLOR_WHITE
    );
    if (err) return err;

    // X-axis label
    x_axis_transform = MAT4_IDENT;
    mat4_scale(&x_axis_transform, &((struct vec3) { 1.0f, 0.01f, 0.01f }));
    mat4_translate(&x_axis_transform, &((struct vec3) { 0.5f, 0.0f, 0.0f }));

    // Y-axis label
    y_axis_transform = MAT4_IDENT;
    mat4_scale(&y_axis_transform, &((struct vec3) { 0.01f, 1.0f, 0.01f }));
    mat4_translate(&y_axis_transform, &((struct vec3) { 0.0f, 0.5f, 0.0f }));

    // Z-axis label
    z_axis_transform = MAT4_IDENT;
    mat4_scale(&z_axis_transform, &((struct vec3) { 0.01f, 0.01f, 1.0f }));
    mat4_translate(&z_axis_transform, &((struct vec3) { 0.0f, 0.0f, 0.5f }));

    return err;
}

int init_hardcoded_test_chunk(u32 *meshes, u32 mesh_count)
{
    enum rico_error err;

    // Initialize object pool
    err = rico_object_init(RICO_OBJECT_POOL_SIZE);
    if (err) return err;

    //--------------------------------------------------------------------------
    // Create world objects
    //--------------------------------------------------------------------------
    u32 obj_fonttest;
    u32 obj_ground;
    u32 obj_yellow;
    u32 obj_ruler;
    u32 obj_wall1;
    u32 obj_wall2;
    u32 obj_wall3;
    u32 obj_wall4;
    u32 obj_wall5;
    u32 arr_objects[50] = { 0 };

    // World font object
    err = object_create(&obj_fonttest, "World String Test", OBJ_STRING_WORLD,
                        mesh_font_test, tex_font_test, NULL);
    if (err) return err;
    object_trans(obj_fonttest, 0.0f, 2.0f, 0.0f);
    object_scale(obj_fonttest, 1.0f, 1.0f, 1.0f);

    // Screen font object
    err = object_create(&obj_fonttest, "Screen String Test", OBJ_STRING_SCREEN,
                        mesh_font_test, tex_font_test, NULL);
    if (err) return err;
    object_trans(obj_fonttest, -1.0f, 0.0f, 0.0f);

    // Ground
    err = object_create(&obj_ground, "Ground", OBJ_DEFAULT, mesh_default,
                        tex_rock, NULL);
    if (err) return err;
    object_rot_x(obj_ground, -90.0f);
    object_scale(obj_ground, 64.0f, 64.0f, 1.0f);

    // Hello
    err = object_create(&obj_yellow, "Yellow", OBJ_DEFAULT, mesh_default,
                        tex_yellow, NULL);
    if (err) return err;
    object_trans(obj_yellow, 0.0f, 2.0f, -4.0f);
    object_rot_y(obj_yellow, 40.0f);
    object_scale(obj_yellow, 1.0f, 2.0f, 1.0f);

    // Ruler
    err = object_create(&obj_ruler, "Ruler", OBJ_DEFAULT, mesh_default,
                        RICO_TEXTURE_DEFAULT, NULL);
    if (err) return err;
    object_trans(obj_ruler, 0.0f, 1.0f, -3.0f);
    object_scale(obj_ruler, 1.0f, 1.0f, 1.0f);

    // Walls are all the same size for now
    struct vec3 wall_scale = (struct vec3) { 8.0f, 2.5f, 1.0f };

    // Wall front
    err = object_create(&obj_wall1, "wall1", OBJ_DEFAULT, mesh_default,
                        tex_hello, NULL);
    if (err) return err;
    object_trans(obj_wall1, 0.0f, 2.5f, -8.0f);
    object_scale(obj_wall1, wall_scale.x, wall_scale.y, wall_scale.z);

    // Wall left
    err = object_create(&obj_wall2, "wall2", OBJ_DEFAULT, mesh_default,
                        tex_hello, NULL);
    if (err) return err;
    object_trans(obj_wall2, -8.0f, 2.5f, 0.0f);
    object_scale(obj_wall2, wall_scale.x, wall_scale.y, wall_scale.z);

    // Wall back
    err = object_create(&obj_wall3, "wall3", OBJ_DEFAULT, mesh_default,
                        tex_hello, NULL);
    if (err) return err;
    object_trans(obj_wall3, 0.0f, 2.5f, 8.0f);
    object_rot_y(obj_wall3, 180.0f);
    object_scale(obj_wall3, wall_scale.x, wall_scale.y, wall_scale.z);

    // Wall right
    err = object_create(&obj_wall4, "wall4", OBJ_DEFAULT, mesh_default,
                        tex_hello, NULL);
    if (err) return err;
    object_trans(obj_wall4, 8.0f, 2.5f, 0.0f);
    object_rot_y(obj_wall4, -90.0f);
    object_scale(obj_wall4, wall_scale.x, wall_scale.y, wall_scale.z);

    // Wall five
    err = object_create(&obj_wall5, "wall5", OBJ_DEFAULT, mesh_default,
                        tex_hello, NULL);
    if (err) return err;
    object_trans(obj_wall5, 4.0f, 2.5f, 0.0f);
    object_rot_y(obj_wall5, -90.0f);
    object_scale(obj_wall5, wall_scale.x, wall_scale.y, wall_scale.z);

    {
        for (u32 i = 0; i < mesh_count; i++)
        {
            err = object_create(&arr_objects[i], mesh_name(meshes[i]),
                                OBJ_DEFAULT, meshes[i], RICO_TEXTURE_DEFAULT,
                                mesh_bbox(meshes[i]));
            if (err) return err;

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

    err = chunk_init(0, "my_first_chunk", chunk_tex_pool->count,
                     chunk_mesh_pool->count, chunk_obj_pool->count,
                     chunk_tex_pool, chunk_mesh_pool, chunk_obj_pool, &chunkA);
    if (err) return err;

    struct rico_file file;
    err = rico_file_open_write(&file, "../res/chunks/cereal.bin", 0);
    if (err) return err;

    err = rico_serialize(&chunkA, &file);
    rico_file_close(&file);

    return err;
}

void select_obj(u32 handle)
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

void selected_translate(struct camera *camera, const struct vec3 *offset)
{
    struct rico_object *obj = object_fetch(selected_handle);

    if (vec3_equals(offset, &VEC3_ZERO))
    {
        if (camera->locked && obj->type != OBJ_STRING_SCREEN)
        {
            vec3_add(&camera->position, &obj->trans);
        }
        obj->trans = VEC3_ZERO;
    }
    else
    {
        if (camera->locked && obj->type != OBJ_STRING_SCREEN)
        {
            vec3_sub(&camera->position, offset);
        }
        vec3_add(&obj->trans, offset);
    }
}

void selected_rotate(const struct vec3 *offset)
{
    if (vec3_equals(offset, &VEC3_ZERO))
    {
        object_fetch(selected_handle)->rot = VEC3_ZERO;
    }
    else
    {
        struct vec3 *rot = &object_fetch(selected_handle)->rot;
        vec3_add(rot, offset);
    }
}

int selected_duplicate()
{
    enum rico_error err;
    struct rico_object *selected = object_fetch(selected_handle);

    u32 newObj;
    err = object_create(&newObj, "Duplicate", selected->type, selected->mesh,
                        selected->texture, mesh_bbox(selected->mesh));
    if (err) return err;

    struct rico_object *new_obj = object_fetch(newObj);
    if (newObj)
    {
        new_obj->trans = selected->trans;
        new_obj->rot = selected->rot;
        new_obj->scale = selected->scale;
    }
    select_obj(newObj);

    return err;
}

void selected_delete()
{
    u32 handle = selected_handle;
    select_prev_obj();
    object_free(handle);
}

//TODO: Put this somewhere reasonable (e.g. lighting module)
static struct col4 ambient = { 0.7f, 0.6f, 0.4f, 1.0f };

void glref_update(GLfloat dt, bool ambient_light)
{
    //--------------------------------------------------------------------------
    // Update uniforms
    //--------------------------------------------------------------------------
    glUseProgram(prog_default->prog_id);
    glUniform1f(prog_default->u_time, dt);

    if (ambient_light)
        glUniform4fv(prog_default->u_ambient, 1, (const GLfloat *)&ambient);
    else
        glUniform4fv(prog_default->u_ambient, 1, (const GLfloat *)&VEC3_UNIT);

    glUseProgram(0);
}

int glref_debuginfo(const char *str, struct col4 color)
{
    enum rico_error err;

    if (obj_debuginfostring)
        object_free(obj_debuginfostring);

    u32 text_mesh;
    u32 text_tex;
    err = font_render(font, 0, 0, color, str, "debug_info_string", &text_mesh,
                      &text_tex);
    if (err) return err;

    err = object_create(&obj_debuginfostring, "DEBUG_INFO", OBJ_STRING_SCREEN,
                        text_mesh, text_tex, NULL);
    if (err) return err;

    object_trans(obj_debuginfostring, -1.0f, 1.0f, 0.0f);

    return err;
}

void glref_render(struct camera *camera)
{
    //--------------------------------------------------------------------------
    // Render objects
    //--------------------------------------------------------------------------
    object_render_type(OBJ_DEFAULT, prog_default, camera);
    object_render_type(OBJ_STRING_WORLD, prog_default, camera);

    //--------------------------------------------------------------------------
    // Axes labels (bboxes)
    //--------------------------------------------------------------------------
    bbox_render_color(&axis_bbox, camera, &x_axis_transform, COLOR_RED);
    bbox_render_color(&axis_bbox, camera, &y_axis_transform, COLOR_GREEN);
    bbox_render_color(&axis_bbox, camera, &z_axis_transform, COLOR_BLUE);

    object_render_type(OBJ_STRING_SCREEN, prog_default, camera);
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