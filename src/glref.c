//TODO: Implement better camera with position + lookat. Is that necessary?
//      Maybe it's easy to derive lookat when I need it? Probably not..
//struct vec3 camera_right = {  }

//global GLuint vao;
//global GLuint vbos[2];

global u32 selected_handle = 0;

global struct program_default *prog_default;
global struct program_primitive *prog_primitive;

//global u32 font;
global u32 tex_font_test;
global u32 mesh_font_test;
global u32 tex_grass;
global u32 tex_rock;
global u32 tex_hello;
global u32 tex_yellow;

global struct bbox axis_bbox;

global struct mat4 x_axis_transform;
global struct mat4 y_axis_transform;
global struct mat4 z_axis_transform;

int init_glref()
{
    enum rico_error err;

    //--------------------------------------------------------------------------
    // Initialize fonts
    //--------------------------------------------------------------------------
    // TODO: Add error handling to make_font()
    //font_init("font/courier_new.bff", &font);

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
    /*err = texture_load_file(&tex_grass, "grass", GL_TEXTURE_2D,
                            "texture/grass.tga", 32);
    if (err) return err;

    err = texture_load_file(&tex_rock, "bricks", GL_TEXTURE_2D,
                            "texture/clean_bricks.tga", 32);
    if (err) return err;

    err = texture_load_file(&tex_hello, "hello", GL_TEXTURE_2D,
                            "texture/hello.tga", 32);
    if (err) return err;

    err = texture_load_file(&tex_yellow, "yellow", GL_TEXTURE_2D,
                            "texture/fake_yellow.tga", 32);
    if (err) return err;*/

    //--------------------------------------------------------------------------
    // Create axis label bboxes
    //--------------------------------------------------------------------------
    err = bbox_init(&axis_bbox, "Axis BBox",
                    (struct vec3) { -0.5f, -0.5f, -0.5f },
                    (struct vec3) {  0.5f,  0.5f,  0.5f }, COLOR_WHITE);
    if (err) return err;

    // X-axis label
    x_axis_transform = MAT4_IDENT;
    mat4_scale(&x_axis_transform, &(struct vec3) { 1.0f, 0.01f, 0.01f });
    mat4_translate(&x_axis_transform, &(struct vec3) { 0.5f, 0.0f, 0.0f });

    // Y-axis label
    y_axis_transform = MAT4_IDENT;
    mat4_scale(&y_axis_transform, &(struct vec3) { 0.01f, 1.0f, 0.01f });
    mat4_translate(&y_axis_transform, &(struct vec3) { 0.0f, 0.5f, 0.0f });

    // Z-axis label
    z_axis_transform = MAT4_IDENT;
    mat4_scale(&z_axis_transform, &(struct vec3) { 0.01f, 0.01f, 1.0f });
    mat4_translate(&z_axis_transform, &(struct vec3) { 0.0f, 0.0f, 0.5f });

    return err;
}

int create_obj()
{
    enum rico_error err;

    // TODO: Prompt user for object name
    const char *name = "new_obj";
    u32 new_obj;

    // Create new object with default properties
    err = object_create(&new_obj, name, OBJ_STATIC, RICO_DEFAULT_MESH,
                        RICO_DEFAULT_MATERIAL, NULL, true);
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

    if (v3_equals(offset, &VEC3_ZERO))
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
            camera_translate_world(camera, offset);
        }
        object_trans(selected_handle, offset);
    }

    object_print(selected_handle, STR_SLOT_SELECTED_OBJ);
}

void selected_rotate(const struct vec3 *offset)
{
    if (!selected_handle)
        return;

    if (v3_equals(offset, &VEC3_ZERO))
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

    if (v3_equals(offset, &VEC3_ZERO))
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
    RICO_ASSERT(axis_bbox.uid.uid != UID_NULL);
    prim_draw_bbox_color(&axis_bbox, &x_axis_transform, &COLOR_RED);
    prim_draw_bbox_color(&axis_bbox, &y_axis_transform, &COLOR_GREEN);
    prim_draw_bbox_color(&axis_bbox, &z_axis_transform, &COLOR_BLUE);

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