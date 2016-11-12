#ifndef RICO_OBJ_H
#define RICO_OBJ_H

#include "geom.h"
#include "rico_mesh.h"
#include "rico_uid.h"
#include <GL/gl3w.h>

struct rico_obj {
    struct rico_uid uid;

    //TODO: Refactor into rico_transform
    //TODO: Animation
    struct vec4 trans;
    struct vec4 rot;
    struct vec4 scale;

    //TODO: Support multiple meshes
    uint32 mesh;

    //TODO: Support multiple textures (per mesh?)
    uint32 texture;

    struct bbox bbox;
};

//Cleanup: Dunno if there are needed outside of this CU
//extern struct rico_obj rico_obj_pool[];
//extern uint32 next_uid;

int rico_object_init(uint32 pool_size);
int rico_obj_create(const char *name, uint32 mesh, uint32 texture,
                    const struct bbox *bbox, uint32 *_handle);
struct rico_obj *rico_obj_fetch(uint32 handle);
void rico_obj_translate(struct vec4 v);
void rico_obj_translate(struct vec4 v);
void rico_obj_translate(struct vec4 v);
uint32 rico_obj_next(uint32 handle);
uint32 rico_obj_prev(uint32 handle);
void rico_obj_render(uint32 handle);
void rico_obj_free(uint32 *handle);

#endif // RICO_OBJ_H