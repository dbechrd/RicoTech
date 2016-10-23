#ifndef RICO_OBJ_H
#define RICO_OBJ_H

#include "geom.h"
#include "mesh.h"
#include <GL/gl3w.h>

struct rico_obj {
    //TODO: Should this be a rico_uid with a string name?
    uint32 uid;

    //TODO: Refactor into rico_transform
    //TODO: Animation
    struct vec4 trans;
    struct vec4 rot;
    struct vec4 scale;

    //TODO: Support multiple meshes
    struct rico_mesh *mesh;
};

//Cleanup: Dunno if there are needed outside of this CU
//extern struct rico_obj rico_obj_pool[];
//extern uint32 next_uid;

struct rico_obj *make_rico_obj();
void rico_obj_render(const struct rico_obj *obj);

static inline void free_rico_obj(struct rico_obj **obj)
{
    (*obj)->uid = 0;
    *obj = NULL;
}

#endif // RICO_OBJ_H