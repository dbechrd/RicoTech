#include "rico_object.h"
#include "const.h"
#include "rico_pool.h"
#include "camera.h"
#include "rico_cereal.h"
#include "program.h"
#include "rico_font.h"
#include <malloc.h>

struct rico_object {
    struct rico_uid uid;
    enum rico_obj_type type;

    //TODO: Refactor into rico_transform
    //TODO: Animation
    struct vec3 trans;
    struct vec3 rot;
    struct vec3 scale;
    //struct mat4 transform;

    //TODO: Support multiple meshes
    u32 mesh;

    //TODO: Support multiple textures (per mesh?)
    u32 texture;

    struct bbox bbox;
};

const char *rico_obj_type_string[] = {
    RICO_OBJ_TYPES(GEN_STRING)
};

u32 RICO_OBJECT_DEFAULT = 0;
static struct rico_pool *objects;

int rico_object_init(u32 pool_size)
{
    objects = calloc(1, sizeof(*objects));
    return pool_init("Objects", pool_size, sizeof(struct rico_object), 0,
                     objects);
}

int object_create(u32 *_handle, const char *name, enum rico_obj_type type,
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

int object_copy(u32 *_handle, u32 handle, const char *name)
{
    enum rico_error err;
    struct rico_object *obj = pool_read(objects, handle);

    // Create new object with same mesh / texture
    err = object_create(_handle, name, obj->type, obj->mesh, obj->texture,
                        NULL);
    if (err) return err;

    // Copy transform
    object_trans_set(*_handle, &obj->trans);
    object_rot_set(*_handle, &obj->rot);
    object_scale_set(*_handle, &obj->scale);

    return err;
}

void object_free(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);

#ifdef RICO_DEBUG_INFO
    printf("[Object] Free %s\n", obj->uid.name);
#endif

    mesh_free(obj->mesh);
    texture_free(obj->texture);

    obj->uid.uid = UID_NULL;
    pool_free(objects, handle);
}

void object_free_all()
{
    for (int i = objects->active - 1; i >= 0; --i)
    {
        object_free(objects->handles[i]);
    }
}

u32 object_next(u32 handle)
{
    return pool_next(objects, handle);
}

u32 object_prev(u32 handle)
{
    return pool_prev(objects, handle);
}

enum rico_obj_type object_type_get(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    return obj->type;
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

void object_trans(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    vec3_add(&obj->trans, v);
}

const struct vec3 *object_trans_get(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    return &obj->trans;
}

void object_trans_set(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->trans = *v;
}

void object_rot(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    vec3_add(&obj->rot, v);
}

void object_rot_set(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot = *v;
}

const struct vec3 *object_rot_get(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    return &obj->rot;
}

void object_rot_x(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.x += deg;
}

void object_rot_x_set(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.x = deg;
}

void object_rot_y(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.y += deg;
}

void object_rot_y_set(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.y = deg;
}

void object_rot_z(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.z += deg;
}

void object_rot_z_set(u32 handle, float deg)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->rot.z = deg;
}

void object_scale(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    vec3_scale(&obj->scale, v);
}

void object_scale_set(u32 handle, const struct vec3 *v)
{
    struct rico_object *obj = pool_read(objects, handle);
    obj->scale = *v;
}

const struct vec3 *object_scale_get(u32 handle)
{
    struct rico_object *obj = pool_read(objects, handle);
    return &obj->scale;
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
    bbox_render(&(obj->bbox), camera, &model_matrix);
}

void object_render(u32 handle, const struct program_default *prog,
                   const struct camera *camera)
{
    object_render_direct(pool_read(objects, handle), prog, camera);
}

void object_render_type(enum rico_obj_type type,
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

int object_print(u32 handle, enum rico_string_slot slot)
{
    enum rico_error err;

    // Print to screen
    char *buf = object_serialize_str(handle);
    err = string_init(rico_string_slot_string[slot], slot, 0, 0,
                      COLOR_GRAY_HIGHLIGHT, 0, RICO_FONT_DEFAULT, buf);
    if (err) goto cleanup;

    // Print to stdout
#ifdef RICO_DEBUG_OBJECT
    if (!handle) {
        printf("[Object 0] NULL\n");
        return;
    }

    struct rico_object *obj = pool_read(objects, handle);
    if (obj)
        printf("[Object %d][uid %d] %s\n", handle, obj->uid.uid, obj->uid.name);
#else
    UNUSED(handle);
#endif

cleanup:
    free(buf);
    return err;
}

char *object_serialize_str(u32 handle)
{
    char *buf = calloc(1, 256);

    if (!handle)
    {
        sprintf(buf,
            "    UID: %d\n" \
            "   Name: --\n" \
            "   Type: --\n" \
            "  Trans: --\n" \
            "    Rot: --\n" \
            "  Scale: --\n" \
            "   Mesh: --\n" \
            "Texture: --\n",
            UID_NULL);
    }
    else
    {
        struct rico_object *obj = pool_read(objects, handle);
        sprintf(buf,
            "    UID: %d\n"       \
            "   Name: %s\n"       \
            "   Type: %s\n"       \
            "  Trans: %f %f %f\n" \
            "    Rot: %f %f %f\n" \
            "  Scale: %f %f %f\n" \
            "   Mesh: %d\n"       \
            "Texture: %d\n",
            obj->uid.uid,
            obj->uid.name,
            rico_obj_type_string[obj->type],
            obj->trans.x, obj->trans.y, obj->trans.z,
            obj->rot.x,   obj->rot.y,   obj->rot.z,
            obj->scale.x, obj->scale.y, obj->scale.z,
            obj->mesh,
            obj->texture);

    }
    return buf;
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