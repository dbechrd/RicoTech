#include "rico_object.h"
#include "const.h"
#include "rico_pool.h"
#include "camera.h"
#include "rico_cereal.h"
#include "program.h"
#include <malloc.h>

u32 RICO_OBJECT_DEFAULT = 0;
static struct rico_pool *objects;

int rico_object_init(u32 pool_size)
{
    objects = calloc(1, sizeof(*objects));
    return pool_init("Objects", pool_size, sizeof(struct rico_object),
                     objects);
}

int object_create(u32 *_handle, const char *name, enum rico_object_type type,
                  u32 mesh, u32 texture, const struct bbox *bbox)
{
#ifdef RICO_DEBUG_OBJECT
    printf("[Object] Init %s\n", name);
#endif

    enum rico_error err;
    *_handle = RICO_OBJECT_DEFAULT;

    err = pool_alloc(objects, _handle);
    if (err) return err;

    struct rico_object *obj = pool_read(objects, *_handle);

    uid_init(&obj->uid, RICO_UID_OBJECT, name);
    obj->type = type;
    obj->scale = VEC3_UNIT;
    obj->mesh = mesh_request(mesh);
    obj->texture = texture_request(texture);

    //HACK: Use mesh bbox if none specified
    obj->bbox = (bbox != NULL) ? *bbox : *mesh_bbox(mesh);

    if (type == OBJ_STRING_SCREEN)
        obj->scale = VEC3_SCALE_ASPECT;

    return err;
}

void object_free(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);

#ifdef RICO_DEBUG_INFO
    printf("[Object] Free %s\n", obj->uid.name);
#endif

    obj->uid = UID_NULL;
    mesh_free(obj->mesh);
    texture_free(obj->texture);
    pool_free(objects, handle);
}

void object_free_all()
{
    for (int i = objects->active; i > 0; --i)
    {
        object_free(objects->handles[i]);
    }
}

//HACK: DANGER WILL ROBINSON!!!
struct rico_object *object_fetch(u32 handle)
{
    return pool_read(objects, handle);
}

u32 object_next(u32 handle)
{
    return pool_next(objects, handle);
}

u32 object_prev(u32 handle)
{
    return pool_prev(objects, handle);
}

void object_select(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->bbox.wireframe = false;
}

void object_deselect(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->bbox.wireframe = true;
}

void object_trans(u32 handle, float x, float y, float z)
{
    struct rico_object *obj = pool_read(objects, handle);
    //union mat4_translate(&obj->transform, (struct vec3) { x, y, z });
    obj->trans = (struct vec3) { x, y, z };
}

void object_rot_x(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    //mat4_rotx(&obj->transform, deg);
    obj->rot.x = deg;
}

void object_rot_y(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    //mat4_roty(&obj->transform, deg);
    obj->rot.y = deg;
}

void object_rot_z(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    //mat4_rotz(&obj->transform, deg);
    obj->rot.z = deg;
}

void object_scale(u32 handle, float x, float y, float z)
{
    struct rico_object *obj = pool_read(objects, handle);
    //mat4_scale(&obj->transform, (struct vec3) { x, y, z });
    obj->scale = (struct vec3) { x, y, z };
}

static void object_render_direct(const struct rico_object *obj,
                                 const struct program_default *prog,
                                 const struct camera *camera)
{
    glPolygonMode(GL_FRONT_AND_BACK, camera->fill_mode);
    glUseProgram(prog->prog_id);

    // Model transform
    struct mat4 proj_matrix;
    struct mat4 view_matrix;
    struct mat4 model_matrix;

    //HACK: Order of these operations might not always be the same.. should
    //      probably just store the transformation matrix directly rather than
    //      trying to figure out which order to do what.
    model_matrix = MAT4_IDENT;
    mat4_translate(&model_matrix, &obj->trans);
    mat4_rotx(&model_matrix, obj->rot.x);
    mat4_roty(&model_matrix, obj->rot.y);
    mat4_rotz(&model_matrix, obj->rot.z);
    mat4_scale(&model_matrix, &obj->scale);

    if (obj->type == OBJ_DEFAULT)
    {
        proj_matrix = camera->proj_matrix;
        view_matrix = camera->view_matrix;

        // HACK: This only works when object is uniformly scaled on X/Y plane.
        /* TODO: UV scaling in general only works when object is uniformly
               scaled. Maybe I should only allow textured objects to be
               uniformly scaled?
        */
        // UV-coord scale
        glUniform2f(prog->u_scale_uv, obj->scale.x, obj->scale.y);
    }
    else if (obj->type == OBJ_STRING_WORLD)
    {
        proj_matrix = camera->proj_matrix;
        view_matrix = camera->view_matrix;

        glUniform2f(prog->u_scale_uv, 1.0f, 1.0f);
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
    //bbox_render(&(obj->bbox), camera, &model_matrix);
}

void object_render(u32 handle, const struct program_default *prog,
                   const struct camera *camera)
{
    object_render_direct(pool_read(objects, handle), prog, camera);
}

void object_render_type(enum rico_object_type type,
                        const struct program_default *prog,
                        const struct camera *camera)
{
    struct rico_object *obj;
    for (u32 i = 0; i < objects->active; ++i)
    {
        obj = pool_read(objects, objects->handles[i]);
        if (obj->type == type)
        {
            object_render_direct(obj, prog, camera);
        }
    }
}

int object_serialize_0(const void *handle, const struct rico_file *file)
{
    const struct rico_object *obj = handle;
    fwrite(&obj->type,    sizeof(obj->type),    1, file->fs);
    fwrite(&obj->trans,   sizeof(obj->trans),   1, file->fs);
    fwrite(&obj->rot,     sizeof(obj->rot),     1, file->fs);
    fwrite(&obj->scale,   sizeof(obj->scale),   1, file->fs);
    fwrite(&obj->mesh,    sizeof(obj->mesh),    1, file->fs);
    fwrite(&obj->texture, sizeof(obj->texture), 1, file->fs);
    rico_serialize(&obj->bbox, file);
    return SUCCESS;
}

int object_deserialize_0(void *_handle, const struct rico_file *file)
{
    struct rico_object *obj = _handle;
    fread(&obj->type,    sizeof(obj->type),    1, file->fs);
    fread(&obj->trans,   sizeof(obj->trans),   1, file->fs);
    fread(&obj->rot,     sizeof(obj->rot),     1, file->fs);
    fread(&obj->scale,   sizeof(obj->scale),   1, file->fs);
    fread(&obj->mesh,    sizeof(obj->mesh),    1, file->fs);
    fread(&obj->texture, sizeof(obj->texture), 1, file->fs);
    rico_deserialize(&obj->bbox, file);
    return SUCCESS;
}

struct rico_pool *object_pool_get_unsafe()
{
    return objects;
}

void object_pool_set_unsafe(struct rico_pool *pool)
{
    RICO_ASSERT(!objects);
    objects = pool;
}