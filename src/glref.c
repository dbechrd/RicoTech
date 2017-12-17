//TODO: Implement better camera with position + lookat. Is that necessary?
//      Maybe it's easy to derive lookat when I need it? Probably not..
//struct vec3 camera_right = {  }

//global GLuint vao;
//global GLuint vbos[2];

global struct rico_object *selected_obj;

global struct program_default *prog_default;
global struct program_primitive *prog_primitive;

//global u32 font;
//global u32 tex_font_test;
//global u32 mesh_font_test;
//global u32 tex_grass;
//global u32 tex_rock;
//global u32 tex_hello;
//global u32 tex_yellow;

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

    // Create new object with default properties
    struct rico_object *new_obj;
    err = chunk_alloc(&new_obj, chunk_active, RICO_HND_OBJECT);
    if (err) return err;
    err = object_init(new_obj, name, OBJ_STATIC, ID_NULL, ID_NULL, NULL);
    if (err) return err;

    // Select new object
    select_obj(new_obj);
    return err;
}

void recalculate_all_bbox()
{
    object_bbox_recalculate_all(chunk_active);

    // Reselect current object
    if (selected_obj)
        object_select(selected_obj);
}

void select_obj(struct rico_object *object)
{
    if (selected_obj == object)
        return;

    // Deselect current object
    if (selected_obj)
        object_deselect(selected_obj);

    // Select requested object
    selected_obj = object;
    if (selected_obj)
        object_select(selected_obj);

    selected_print();
}

void select_next_obj()
{
    select_obj(pool_next(chunk_active->pools[RICO_HND_OBJECT], selected_obj));
}

void select_prev_obj()
{
    select_obj(pool_prev(chunk_active->pools[RICO_HND_OBJECT], selected_obj));
}

void selected_print()
{
    // Print select object's properties
    object_print(selected_obj);
}

void selected_translate(struct camera *camera, const struct vec3 *offset)
{
    if (!selected_obj)
        return;

    bool selectable = object_selectable(selected_obj);

    if (v3_equals(offset, &VEC3_ZERO))
    {
        if (camera->locked && selectable)
        {
            camera->position = VEC3_ZERO;
        }
        object_trans_set(selected_obj, &VEC3_ZERO);
    }
    else
    {
        if (camera->locked && selectable)
        {
            camera_translate_world(camera, offset);
        }
        object_trans(selected_obj, offset);
    }

    object_print(selected_obj);
}

void selected_rotate(const struct vec3 *offset)
{
    if (!selected_obj)
        return;

    if (v3_equals(offset, &VEC3_ZERO))
    {
        object_rot_set(selected_obj, &VEC3_ZERO);
    }
    else
    {
        object_rot(selected_obj, offset);
    }

    object_print(selected_obj);
}

void selected_scale(const struct vec3 *offset)
{
    if (!selected_obj)
        return;

    if (v3_equals(offset, &VEC3_ZERO))
    {
        object_scale_set(selected_obj, &VEC3_ONE);
    }
    else
    {
        object_scale(selected_obj, offset);
    }

    object_print(selected_obj);
}

void selected_mesh_next()
{
    if (!selected_obj)
        return;

    object_mesh_set(selected_obj, chunk_next_id(selected_obj->hnd.chunk,
                                                selected_obj->mesh_id));
    object_select(selected_obj);
    object_print(selected_obj);
}

void selected_mesh_prev()
{
    if (!selected_obj)
        return;

    object_mesh_set(selected_obj, chunk_prev_id(selected_obj->hnd.chunk,
                                                selected_obj->mesh_id));
    object_select(selected_obj);
    object_print(selected_obj);
}

void selected_bbox_reset()
{
    if (!selected_obj)
        return;

    object_bbox_set(selected_obj, NULL);
    object_select(selected_obj);
    object_print(selected_obj);
}

int selected_duplicate()
{
    if (!selected_obj)
        return SUCCESS;

    enum rico_error err;

    struct rico_object *new_obj;
    err = chunk_alloc(&new_obj, chunk_active, RICO_HND_OBJECT);
    if (err) return err;
    err = object_copy(new_obj, selected_obj, "Duplicate");
    if (err) return err;

    select_obj(new_obj);
    return err;
}

void selected_delete()
{
    if (!selected_obj) return;
    if (!object_selectable(selected_obj)) return;

    struct rico_object *to_delete = selected_obj;
    select_prev_obj();  // NOTE: This assumes that prev_obj(first) == NULL
    object_free(to_delete);
}

void glref_update(r64 dt)
{
    //--------------------------------------------------------------------------
    // Update uniforms
    //--------------------------------------------------------------------------
    glUseProgram(prog_default->prog_id);
    glUniform1f(prog_default->u_time, (r32)dt);
    glUseProgram(0);
}

void glref_render(struct camera *camera)
{
    //--------------------------------------------------------------------------
    // Render objects
    //--------------------------------------------------------------------------
    object_render_type(chunk_active,    OBJ_STATIC, prog_default, camera);
    object_render_type(chunk_transient, OBJ_STATIC, prog_default, camera);
    object_render_type(chunk_active,    OBJ_STRING_WORLD, prog_default, camera);
    object_render_type(chunk_transient, OBJ_STRING_WORLD, prog_default, camera);

    //--------------------------------------------------------------------------
    // Axes labels (bboxes)
    //--------------------------------------------------------------------------
    //RICO_ASSERT(axis_bbox.hnd.uid != UID_NULL);
    prim_draw_bbox_color(&axis_bbox, &x_axis_transform, &COLOR_RED);
    prim_draw_bbox_color(&axis_bbox, &y_axis_transform, &COLOR_GREEN);
    prim_draw_bbox_color(&axis_bbox, &z_axis_transform, &COLOR_BLUE);

    object_render_type(chunk_active, OBJ_STRING_SCREEN, prog_default,
                       camera);
    object_render_type(chunk_transient, OBJ_STRING_SCREEN, prog_default,
                       camera);
}
void free_glref()
{
    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    // Free all game objects
    object_free_all(chunk_active);
    object_free_all(chunk_transient);

    //TODO: Free all meshes

    //TODO: Free all textures
    //texture_free(tex_grass);
    //texture_free(tex_rock);
    //texture_free(tex_hello);

    //TODO: Free all programs
    free_program_default(&prog_default);
}
