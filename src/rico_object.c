#include "rico_object.h"
#include "const.h"
#include "rico_pool.h"
#include "camera.h"

uint32 RICO_OBJECT_DEFAULT = 0;
static struct rico_pool *objects;

static void object_render_direct(const struct rico_object *obj,
                                 const struct program_default *prog);

int object_init(uint32 pool_size)
{
    objects = calloc(1, sizeof(*objects));
    return pool_init("Objects", pool_size, sizeof(struct rico_object),
                     objects);
}

int object_create(const char *name, enum rico_object_type type, uint32 mesh,
                  uint32 texture, const struct bbox *bbox, uint32 *_handle)
{
    int err;
    *_handle = RICO_OBJECT_DEFAULT;

#ifdef RICO_DEBUG_INFO
    printf("[Object] Creating %s\n", name);
#endif

    err = pool_alloc(objects, _handle);
    if (err) return err;

    struct rico_object *obj = pool_read(objects, *_handle);

    //TODO: Should default W component be 0 or 1?
    uid_init(name, &obj->uid);
    obj->type = type;
    obj->scale = (struct vec4) { 1.0f, 1.0f, 1.0f, 1.0f };
    obj->mesh = mesh;
    obj->texture = texture;
    //HACK: Use mesh bbox if none specified
    obj->bbox = (bbox != NULL) ? *bbox : *mesh_bbox(mesh);

    return err;
}

void object_free(uint32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->uid = UID_NULL;
    pool_free(objects, handle);
    handle = RICO_OBJECT_DEFAULT;
}

void object_free_all()
{
    for (int i = objects->active; i < 0; --i)
    {
        object_free(objects->handles[i]);
    }
}

//HACK: DANGER WILL ROBINSON!!!
struct rico_object *object_fetch(uint32 handle)
{
    return pool_read(objects, handle);
}

uint32 object_next(uint32 handle)
{
    return pool_next(objects, handle);
}

uint32 object_prev(uint32 handle)
{
    return pool_prev(objects, handle);
}

void object_select(uint32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->bbox.color = COLOR_RED_HIGHLIGHT;
    obj->bbox.wireframe = false;
}

void object_deselect(uint32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->bbox.color = COLOR_GRAY_HIGHLIGHT;
    obj->bbox.wireframe = true;
}

void object_trans(uint32 handle, float x, float y, float z)
{
    struct rico_object *obj = pool_read(objects, handle);
    //mat4_translate(&obj->transform, (struct vec4) { x, y, z, 1.0f });
    obj->trans = (struct vec4) { x, y, z, 1.0f };
}

void object_rot_x(uint32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    //mat4_rotx(&obj->transform, deg);
    obj->rot.x = deg;
}

void object_rot_y(uint32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    //mat4_roty(&obj->transform, deg);
    obj->rot.y = deg;
}

void object_rot_z(uint32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    //mat4_rotz(&obj->transform, deg);
    obj->rot.z = deg;
}

void object_scale(uint32 handle, float x, float y, float z)
{
    struct rico_object *obj = pool_read(objects, handle);
    //mat4_scale(&obj->transform, (struct vec4) { x, y, z, 1.0f });
    obj->scale = (struct vec4) { x, y, z, 1.0f };
}

void object_render(uint32 handle, const struct program_default *prog)
{
    object_render_direct(pool_read(objects, handle), prog);
}

static void object_render_direct(const struct rico_object *obj,
                                 const struct program_default *prog)
{
    glPolygonMode(GL_FRONT_AND_BACK, view_camera.fill_mode);
    glUseProgram(prog->prog_id);

    // Model transform
    struct mat4 proj_matrix;
    struct mat4 view_matrix;
    struct mat4 model_matrix;

    //HACK: Order of these operations might not always be the same.. should
    //      probably just store the transformation matrix directly rather than
    //      trying to figure out which order to do what.
    mat4_ident(&model_matrix);
    mat4_translate(&model_matrix, obj->trans);
    mat4_rotx(&model_matrix, obj->rot.x);
    mat4_roty(&model_matrix, obj->rot.y);
    mat4_rotz(&model_matrix, obj->rot.z);
    mat4_scale(&model_matrix, obj->scale);

    if (obj->type == OBJ_DEFAULT || obj->type == OBJ_STRING_WORLD)
    {
        proj_matrix = view_camera.proj_matrix;
        view_matrix = view_camera.view_matrix;

        // HACK: This only works when object is uniformly scaled on X/Y plane.
        // TODO: UV scaling in general only works when object is uniformly scaled.
        //       Maybe I should only allow textured objects to be uniformly scaled?
        // UV-coord scale
        glUniform2f(prog->u_scale_uv, obj->scale.x, obj->scale.y);
    }
    else if (obj->type == OBJ_STRING_SCREEN)
    {
        proj_matrix = MAT4_IDENT;
        view_matrix = MAT4_IDENT;

        glUniform2f(prog->u_scale_uv, 1.0f, 1.0f);
    }

    glUniformMatrix4fv(prog->u_proj, 1, GL_TRUE, proj_matrix.a);
    glUniformMatrix4fv(prog->u_view, 1, GL_TRUE, view_matrix.a);
    glUniformMatrix4fv(prog->u_model, 1, GL_TRUE, model_matrix.a);

    // Model texture
    // Note: We don't have to do this every time as long as we make sure
    //       the correct textures are bound before each draw to the texture
    //       index assumed when the program was initialized.
    glUniform1i(prog->u_tex, 0);



    // Bind texture and render mesh
    //
    texture_bind(obj->texture);
    mesh_render(obj->mesh);

    // Clean up
    texture_unbind(obj->texture);
    glUseProgram(0);

    // Render bbox
    bbox_render(&(obj->bbox), &proj_matrix, &view_matrix, &model_matrix);
}

void object_render_type(enum rico_object_type type,
                       const struct program_default *prog)
{
    struct rico_object *obj;
    for (uint32 i = 0; i < objects->active; ++i)
    {
        obj = pool_read(objects, objects->handles[i]);
        if (obj->type == type)
        {
            object_render_direct(obj, prog);
        }
    }
}

struct rico_pool *object_pool_get_unsafe()
{
    return objects;
}

void object_pool_set_unsafe(struct rico_pool *pool)
{
    rico_assert(!objects);
    objects = pool;
}