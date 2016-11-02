#include "rico_obj.h"
#include "const.h"

#define RICO_OBJ_POOL_SIZE 50

//TODO: Allocate from heap pool, not stack
static struct rico_obj rico_obj_pool[RICO_OBJ_POOL_SIZE];

//TODO: Instantiate rico_obj_pool[0] (handle 0) with a special default object
//      that can be used to visually represent a NULL object in-game
static uint32 next_handle = 1;

//TODO: Move this to some global UID handler
static uint32 next_uid = 1;

struct rico_obj *rico_obj_create(const char *name, const struct rico_mesh *mesh,
                                 const struct bbox *bbox)
{
    //TODO: Handle out-of-memory
    //TODO: Implement reuse of pool objects
    if (next_handle >= RICO_OBJ_POOL_SIZE)
    {
        fprintf(stderr, "Out of memory: Object pool exceeded max size of %d.\n",
                RICO_OBJ_POOL_SIZE);
        return NULL;
    }

    struct rico_obj *obj = &rico_obj_pool[next_handle];
    next_handle++;

    //TODO: Should default W component be 0 or 1?
    uid_init(name, &obj->uid);
    obj->handle = next_handle;
    obj->scale = (struct vec4) { 1.0f, 1.0f, 1.0f, 1.0f };
    obj->mesh = mesh;
    obj->bbox = *bbox;
    return obj;
}

struct rico_obj *rico_obj_fetch(uint32 handle)
{
    return &rico_obj_pool[handle];
}

//HACK: Is iterating all objects really a useful thing to do?
uint32 rico_obj_next(uint32 handle)
{
    return (handle < next_handle - 1) ? ++handle : 0;
}
uint32 rico_obj_prev(uint32 handle)
{
    return (handle > 0) ? --handle : next_handle - 1;
}

void rico_obj_render(const struct rico_obj *obj)
{
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
    mesh_render(obj->mesh, &model_matrix, obj->scale);

    // Render bbox
    bbox_render(&(obj->bbox), &model_matrix);
}