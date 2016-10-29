#ifndef RICO_OBJ_H
#define RICO_OBJ_H

#include "geom.h"
#include "mesh.h"
#include <GL/gl3w.h>

struct rico_obj {
    //TODO: Should this be a rico_uid with a string name?
    uint32 uid;

    //TODO: Create a uid -> handle map for each subsystem?
    uint32 handle;

    //TODO: Refactor into rico_transform
    //TODO: Animation
    struct vec4 trans;
    struct vec4 rot;
    struct vec4 scale;

    //TODO: Replace with handle
    //TODO: Support multiple meshes
    const struct rico_mesh *mesh;

    struct bbox bbox;
};

//Cleanup: Dunno if there are needed outside of this CU
//extern struct rico_obj rico_obj_pool[];
//extern uint32 next_uid;

struct rico_obj *rico_obj_create(const struct rico_mesh *mesh,
                                 const struct bbox *bbox);
struct rico_obj *rico_obj_fetch(uint32 handle);
uint32 rico_obj_next(uint32 handle);
void rico_obj_render(const struct rico_obj *obj);

static inline void free_rico_obj(struct rico_obj **obj)
{
    (*obj)->uid = 0;
    (*obj)->handle = 0;
    *obj = NULL;
}

#endif // RICO_OBJ_H