#include "rico_obj.h"
#include "const.h"
#include "rico_pool.h"

uint32 RICO_OBJECT_DEFAULT = 0;
static struct rico_pool objects;

int rico_object_init(uint32 pool_size)
{
    return pool_init("Objects", pool_size, sizeof(struct rico_obj),
                     &objects);
}

int rico_obj_create(const char *name, uint32 mesh, uint32 texture,
                    const struct bbox *bbox, uint32 *_handle)
{
    int err;
    *_handle = RICO_OBJECT_DEFAULT;

    err = pool_alloc(&objects, _handle);
    if (err) return err;

    struct rico_obj *obj = pool_read(&objects, *_handle);

    //TODO: Should default W component be 0 or 1?
    uid_init(name, &obj->uid);
    obj->scale = (struct vec4) { 1.0f, 1.0f, 1.0f, 1.0f };
    obj->mesh = mesh;
    obj->texture = texture;
    //HACK: Use mesh bbox if none specified
    obj->bbox = (bbox != NULL) ? *bbox : *mesh_bbox(mesh);

    return err;
}

void rico_obj_free(uint32 *handle)
{
    struct rico_obj *obj = pool_read(&objects, *handle);
    obj->uid = UID_NULL;
    pool_free(&objects, handle);
    *handle = RICO_OBJECT_DEFAULT;
}

//HACK: DANGER WILL ROBINSON!!!
struct rico_obj *rico_obj_fetch(uint32 handle)
{
    return pool_read(&objects, handle);
}

//HACK: Is iterating all objects really a useful thing to do?
uint32 rico_obj_next(uint32 handle)
{
    return pool_next(&objects, handle);
    //return (handle < next_handle - 1) ? ++handle : 0;
}
uint32 rico_obj_prev(uint32 handle)
{
    return pool_prev(&objects, handle);
    //return (handle > 0) ? --handle : next_handle - 1;
}

void rico_obj_render(uint32 handle)
{
    struct rico_obj *obj = pool_read(&objects, handle);

    // Model transform
    struct mat4 model_matrix;
    mat4_ident(&model_matrix);

    //HACK: Order of these operations might not always be the same.. should
    //      probably just store the transformation matrix directly rather than
    //      trying to figure out which order to do what.
    mat4_translate(&model_matrix, obj->trans);
    mat4_rotx(&model_matrix, obj->rot.x);
    mat4_roty(&model_matrix, obj->rot.y);
    mat4_rotz(&model_matrix, obj->rot.z);
    mat4_scale(&model_matrix, obj->scale);

    // Render mesh
    mesh_render(obj->mesh, obj->texture, &model_matrix, obj->scale);

    // Render bbox
    bbox_render(&(obj->bbox), &model_matrix);
}