#include "glref.h"
#include "const.h"
#include "geom.h"
#include "util.h"
#include "shader.h"
#include "program.h"
#include "bbox.h"
#include "camera.h"
#include "rico_texture.h"
#include "rico_material.h"
#include "rico_mesh.h"
#include "rico_object.h"
#include "rico_font.h"
#include "rico_chunk.h"
#include "rico_pool.h"
#include "primitives.h"

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
static u32 tex_grass;
static u32 tex_rock;
static u32 tex_hello;
static u32 tex_yellow;

static struct bbox axis_bbox;

static struct mat4 x_axis_transform;
static struct mat4 y_axis_transform;
static struct mat4 z_axis_transform;

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
    err = texture_load_file("grass", GL_TEXTURE_2D, "texture/grass.tga", 32,
                            &tex_grass);
    if (err) return err;
    err = texture_load_file("bricks", GL_TEXTURE_2D, "texture/clean_bricks.tga",
                            32, &tex_rock);
    if (err) return err;
    err = texture_load_file("hello", GL_TEXTURE_2D, "texture/hello.tga", 32,
                            &tex_hello);
    if (err) return err;
    err = texture_load_file("yellow", GL_TEXTURE_2D, "texture/fake_yellow.tga",
                            32, &tex_yellow);
    if (err) return err;

    //--------------------------------------------------------------------------
    // Create 3D strings
    //--------------------------------------------------------------------------
    // Font Test
    err = font_render(font, 0, 0, COLOR_DARK_RED_HIGHLIGHT,
                      "This is a test.\nYay!", "Font test", MESH_OBJ_WORLD,
                      &mesh_font_test, &tex_font_test);
    if (err) return err;

    //--------------------------------------------------------------------------
    // Create axis label bboxes
    //--------------------------------------------------------------------------
    err = bbox_init(&axis_bbox, "Axis BBox",
                    (struct vec3) { -0.5f, -0.5f, -0.5f },
                    (struct vec3) {  0.5f,  0.5f,  0.5f }, COLOR_WHITE);
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

int init_hardcoded_test_chunk()
{
    enum rico_error err;

    //--------------------------------------------------------------------------
    // Create materials
    //--------------------------------------------------------------------------
    u32 material_rock;
    err = material_init("Rock", tex_rock, RICO_TEXTURE_DEFAULT_SPEC, 0.5f,
                        &material_rock);
    if (err) return err;

    //--------------------------------------------------------------------------
    // Create world objects
    //--------------------------------------------------------------------------

    // Ground
    u32 obj_ground;
    err = object_create(&obj_ground, "Ground", OBJ_STATIC, RICO_MESH_DEFAULT,
                        material_rock, NULL, true);
    if (err) return err;
    object_rot_x(obj_ground, -90.0f);
    object_scale(obj_ground, &(struct vec3) { 64.0f, 64.0f, 1.0f });

    // TEST: Create test object for each mesh / primitive
    {
        u32 arr_objects[RICO_MESH_POOL_SIZE] = { 0 };
        u32 i = 0;

        /*
        // Cleanup: Could use mesh_pool_get_unsafe(), but what's really the
        //          the point of this code?
        // Create test object for each loaded mesh
        for (; i < mesh_count; i++)
        {
            err = object_create(&arr_objects[i], mesh_name(meshes[i]),
                                OBJ_STATIC, meshes[i], RICO_MATERIAL_DEFAULT,
                                mesh_bbox(meshes[i]), true);
            if (err) return err;

            // HACK: Don't z-fight ground plane
            object_trans_set(arr_objects[i],
                             &(struct vec3) { 0.0f, EPSILON, 0.0f });

            // HACK: Scale scene 1/10 (for conference room test)
            // object_scale_set(arr_objects[i],
            //                  &(struct vec3) { 0.1f, 0.1f, 0.1f });
        }
        */

        // Create test object for each primitive
        err = object_create(&arr_objects[i], mesh_name(PRIM_SPHERE_MESH),
                            OBJ_STATIC, PRIM_SPHERE_MESH,
                            RICO_MATERIAL_DEFAULT, mesh_bbox(PRIM_SPHERE_MESH),
                            true);
        i++;
        if (err) return err;

        // HACK: Don't z-fight ground plane
        object_trans_set(arr_objects[i], &(struct vec3) { 0.0f, EPSILON, 0.0f });
    }

    //--------------------------------------------------------------------------
    // Save manual chunk
    //--------------------------------------------------------------------------
    struct rico_chunk chunkA;

    err = chunk_init("my_first_chunk", 0,
                     RICO_STRING_POOL_SIZE,
                     RICO_FONT_POOL_SIZE,
                     RICO_TEXTURE_POOL_SIZE,
                     RICO_MATERIAL_POOL_SIZE,
                     RICO_MESH_POOL_SIZE,
                     RICO_OBJECT_POOL_SIZE,
                     &chunkA);
    if (err) return err;

    struct rico_file file;
    err = rico_file_open_write(&file, "../res/chunks/cereal.bin", 0);
    if (err) return err;

    err = rico_serialize(&chunkA, &file);
    rico_file_close(&file);

    return err;
}

int create_obj()
{
    enum rico_error err;

    // TODO: Prompt user for object name
    const char *name = "new_obj";
    u32 new_obj;

    // Create new object with default properties
    err = object_create(&new_obj, name, OBJ_STATIC, RICO_MESH_DEFAULT,
                        RICO_MATERIAL_DEFAULT, NULL, true);
    if (err) return err;

    // Select new object
    select_obj(new_obj);
    return err;
}

void recalculate_all_bbox()
{
    object_bbox_recalculate_all();

    // Reselect current object
    if (selected_handle)
        object_select(selected_handle);
}

void select_obj(u32 handle)
{
    if (selected_handle == handle)
        return;

    // Deselect current object
    if (selected_handle)
        object_deselect(selected_handle);

    // Select requested object
    selected_handle = handle;
    if (selected_handle)
        object_select(selected_handle);

    selected_print();
}

void select_next_obj()
{
    select_obj(object_next(selected_handle));
}

void select_prev_obj()
{
    select_obj(object_prev(selected_handle));
}

void selected_print()
{
    // Print select object's properties
    object_print(selected_handle, STR_SLOT_SELECTED_OBJ);
}

void selected_translate(struct camera *camera, const struct vec3 *offset)
{
    if (!selected_handle)
        return;

    enum rico_obj_type selected_type = object_type_get(selected_handle);

    if (vec3_equals(offset, &VEC3_ZERO))
    {
        if (camera->locked && selected_type != OBJ_STRING_SCREEN)
        {
            camera->position = VEC3_ZERO;
        }
        object_trans_set(selected_handle, &VEC3_ZERO);
    }
    else
    {
        if (camera->locked && selected_type != OBJ_STRING_SCREEN)
        {
            struct vec3 offset_tmp = *offset;
            camera_translate_world(camera, &offset_tmp);
        }
        object_trans(selected_handle, offset);
    }

    object_print(selected_handle, STR_SLOT_SELECTED_OBJ);
}

void selected_rotate(const struct vec3 *offset)
{
    if (!selected_handle)
        return;

    if (vec3_equals(offset, &VEC3_ZERO))
    {
        object_rot_set(selected_handle, &VEC3_ZERO);
    }
    else
    {
        object_rot(selected_handle, offset);
    }

    object_print(selected_handle, STR_SLOT_SELECTED_OBJ);
}

void selected_scale(const struct vec3 *offset)
{
    if (!selected_handle)
        return;

    if (vec3_equals(offset, &VEC3_ZERO))
    {
        object_scale_set(selected_handle, &VEC3_ONE);
    }
    else
    {
        object_scale(selected_handle, offset);
    }

    object_print(selected_handle, STR_SLOT_SELECTED_OBJ);
}

void selected_mesh_next()
{
    if (!selected_handle)
        return;

    object_mesh_next(selected_handle);
    object_select(selected_handle);
    object_print(selected_handle, STR_SLOT_SELECTED_OBJ);
}

void selected_mesh_prev()
{
    if (!selected_handle)
        return;

    object_mesh_prev(selected_handle);
    object_select(selected_handle);
    object_print(selected_handle, STR_SLOT_SELECTED_OBJ);
}

void selected_bbox_reset()
{
    if (!selected_handle)
        return;

    object_bbox_set(selected_handle, NULL);
    object_select(selected_handle);
    object_print(selected_handle, STR_SLOT_SELECTED_OBJ);
}

int selected_duplicate()
{
    if (!selected_handle)
        return SUCCESS;

    enum rico_error err;

    u32 new_obj;
    err = object_copy(&new_obj, selected_handle, "Duplicate");
    if (err) return err;

    select_obj(new_obj);
    return err;
}

void selected_delete()
{
    if (!selected_handle)
        return;

    enum rico_obj_type selected_type = object_type_get(selected_handle);
    if (selected_type == OBJ_NULL || selected_type == OBJ_STRING_SCREEN)
        return;

    u32 prev = object_prev(selected_handle);
    if (prev == selected_handle)
        prev = 0;

    object_free(selected_handle);
    select_obj(prev);
}

void glref_update(r64 dt)
{
    //--------------------------------------------------------------------------
    // Update uniforms
    //--------------------------------------------------------------------------
    glUseProgram(prog_default->prog_id);
    glUniform1f(prog_default->u_time, (r32)dt);
    glUseProgram(0);

    //--------------------------------------------------------------------------
    // Update debug text
    //--------------------------------------------------------------------------
    string_update(dt);
}

void glref_render(struct camera *camera)
{
    //--------------------------------------------------------------------------
    // Render objects
    //--------------------------------------------------------------------------
    object_render_type(OBJ_STATIC, prog_default, camera);
    object_render_type(OBJ_STRING_WORLD, prog_default, camera);

    //--------------------------------------------------------------------------
    // Axes labels (bboxes)
    //--------------------------------------------------------------------------
    bbox_render_color(&axis_bbox, &x_axis_transform, COLOR_RED);
    bbox_render_color(&axis_bbox, &y_axis_transform, COLOR_GREEN);
    bbox_render_color(&axis_bbox, &z_axis_transform, COLOR_BLUE);

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