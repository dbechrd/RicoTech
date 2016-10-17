#include "rico_obj.h"

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
    mesh_render(obj->mesh, &model_matrix);
}