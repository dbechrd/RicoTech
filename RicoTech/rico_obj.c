#include "rico_obj.h"

#define RICO_OBJ_POOL_SIZE 20

struct rico_obj rico_obj_pool[RICO_OBJ_POOL_SIZE];
uint32 next_uid = 1;

//TODO: Should default W component be 0 or 1?
const struct rico_obj RICO_OBJ_DEFAULT = {
    0,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f, 0.0f },
    NULL
};

struct rico_obj *make_rico_obj()
{
    //TODO: Handle out-of-memory
    //TODO: Implement reuse of pool objects
    if (next_uid >= RICO_OBJ_POOL_SIZE)
        return NULL;

    rico_obj_pool[next_uid] = RICO_OBJ_DEFAULT;
    rico_obj_pool[next_uid].uid = next_uid;
    return &rico_obj_pool[next_uid++];
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
}