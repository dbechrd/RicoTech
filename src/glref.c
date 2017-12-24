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

void init_glref()
{
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
    bbox_init(&axis_bbox, (struct vec3) { -0.5f, -0.5f, -0.5f },
                          (struct vec3) {  0.5f,  0.5f,  0.5f }, COLOR_WHITE);

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
}

void create_obj()
{
    // TODO: Prompt user for object name
    const char *name = "new_obj";

    // Create new object with default properties
    u32 new_obj_id = load_object(pack_active, name, OBJ_STATIC, 0, 0, NULL);
    struct rico_object *new_obj = pack_read(pack_active, new_obj_id);

    // Select new object
    select_obj(new_obj);
}

void recalculate_all_bbox()
{
    object_bbox_recalculate_all(pack_active);

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

// TODO: Refactor this out into pack_next(pack, id)
void select_next_obj()
{
    if (pack_active->blobs_used == 0)
        return;

    struct rico_object *obj;

    u32 start = (selected_obj) ? selected_obj->id : 1;
    u32 blob = start;
    do
    {
        blob++;
        if (blob == pack_active->blobs_used)
        {
            select_obj(NULL);
            return;
        }

        if (pack_active->index[blob].type == RICO_HND_OBJECT)
        {
            obj = pack_read(pack_active, blob);
            if (object_selectable(obj))
            {
                select_obj(obj);
                return;
            }
        }
    } while (blob != start);

    select_obj(NULL);
}

// TODO: Refactor this out into pack_prev(pack, id)
void select_prev_obj()
{
    if (pack_active->blobs_used == 0)
        return;

    struct rico_object *obj;

    u32 start = (selected_obj) ? selected_obj->id : pack_active->blobs_used;
    u32 blob = start;
    do
    {
        blob--;
        if (blob == 0)
        {
            select_obj(NULL);
            return;
        }

        if (pack_active->index[blob].type == RICO_HND_OBJECT)
        {
            obj = pack_read(pack_active, blob);
            if (object_selectable(obj))
            {
                select_obj(obj);
                return;
            }
        }
    } while (blob != start);

    select_obj(NULL);
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
#if 0
    if (!selected_obj)
        return;

    object_mesh_set(selected_obj, chunk_next_id(selected_obj->hnd.chunk,
                                                selected_obj->mesh_id));
    object_select(selected_obj);
    object_print(selected_obj);
#endif
}

void selected_mesh_prev()
{
#if 0
    if (!selected_obj)
        return;

    object_mesh_set(selected_obj, chunk_prev_id(selected_obj->hnd.chunk,
                                                selected_obj->mesh_id));
    object_select(selected_obj);
    object_print(selected_obj);
#endif
}

void selected_bbox_reset()
{
    if (!selected_obj)
        return;

    struct rico_mesh *mesh = pack_read(pack_active, selected_obj->mesh_id);
    selected_obj->bbox = mesh->bbox;

    object_select(selected_obj);
    object_print(selected_obj);
}

void selected_duplicate()
{
    if (!selected_obj)
        return;

    // TODO: Prompt user for name
    const char *name = "Duplicate";

    struct rico_object *new_obj = object_copy(pack_active, selected_obj, name);
    select_obj(new_obj);
}

void selected_delete()
{
    if (!selected_obj) return;
    if (!object_selectable(selected_obj)) return;

    struct rico_object *to_delete = selected_obj;
    select_prev_obj();  // NOTE: This assumes that prev_obj(first) == NULL

    // TODO: There is no way to delete from a pack atm.. hmmm.. Will have to
    //       add pack_delete(u32 id) that sets index[id].deleted = true, then
    //       garbage collect / compact the pack whenever it makes sense (e.g.
    //       when out of memory or saving to disk).
    //object_free(to_delete);
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
    object_render_type(pack_active, OBJ_STATIC, prog_default, camera);
    object_render_type(pack_short, OBJ_STATIC, prog_default, camera);
    object_render_type(pack_frame, OBJ_STATIC, prog_default, camera);
    object_render_type(pack_active, OBJ_STRING_WORLD, prog_default, camera);
    object_render_type(pack_short, OBJ_STRING_WORLD, prog_default, camera);
    object_render_type(pack_frame, OBJ_STRING_WORLD, prog_default, camera);

    //--------------------------------------------------------------------------
    // Axes labels (bboxes)
    //--------------------------------------------------------------------------
    //RICO_ASSERT(axis_bbox.hnd.uid != UID_NULL);
    prim_draw_bbox_color(&axis_bbox, &x_axis_transform, &COLOR_RED);
    prim_draw_bbox_color(&axis_bbox, &y_axis_transform, &COLOR_GREEN);
    prim_draw_bbox_color(&axis_bbox, &z_axis_transform, &COLOR_BLUE);

    object_render_type(pack_active, OBJ_STRING_SCREEN, prog_default, camera);
    object_render_type(pack_short, OBJ_STRING_SCREEN, prog_default, camera);
    object_render_type(pack_frame, OBJ_STRING_SCREEN, prog_default, camera);
}
void free_glref()
{
    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    // Free all game objects

    // TODO: What are chunks used for now? How/when do we need to free them?
    //chunk_free(chunk_active);
    //chunk_free(chunk_transient);
    free(pack_frame);
    free(pack_short);
    free(pack_active);
    free(pack_default);

    //TODO: Free all meshes

    //TODO: Free all textures
    //texture_free(tex_grass);
    //texture_free(tex_rock);
    //texture_free(tex_hello);

    //TODO: Free all programs
    free_program_default(&prog_default);
}
