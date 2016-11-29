#ifndef RICO_OBJ_H
#define RICO_OBJ_H

#include "geom.h"
#include "rico_mesh.h"
#include "rico_uid.h"
#include <GL/gl3w.h>

enum rico_object_type {
    OBJ_NULL,
    OBJ_DEFAULT,
    OBJ_STRING_WORLD,
    OBJ_STRING_SCREEN,
    OBJ_ALL
};

struct rico_object {
    struct rico_uid uid;
    enum rico_object_type type;

    //TODO: Refactor into rico_transform
    //TODO: Animation
    struct vec4 trans;
    struct vec4 rot;
    struct vec4 scale;
    //struct mat4 transform;

    //TODO: Support multiple meshes
    uint32 mesh;

    //TODO: Support multiple textures (per mesh?)
    uint32 texture;

    struct bbox bbox;
};

//Cleanup: Dunno if there are needed outside of this CU
//extern struct rico_object object_pool[];
//extern uint32 next_uid;

int object_init(uint32 pool_size);
int object_create(uint32 *_handle, const char *name, enum rico_object_type type,
                  uint32 mesh, uint32 texture, const struct bbox *bbox);
void object_free(uint32 handle);
void object_free_all();
struct rico_object *object_fetch(uint32 handle);
uint32 object_next(uint32 handle);
uint32 object_prev(uint32 handle);
void object_select(uint32 handle);
void object_deselect(uint32 handle);
void object_trans(uint32 handle, float x, float y, float z);
void object_rot_x(uint32 handle, float deg);
void object_rot_y(uint32 handle, float deg);
void object_rot_z(uint32 handle, float deg);
void object_scale(uint32 handle, float x, float y, float z);
void object_render(uint32 handle, const struct program_default *prog);
void object_render_type(enum rico_object_type type,
                        const struct program_default *prog);

int object_serialize(uint32 handle, FILE *fs);
int object_deserialize(uint32 *_handle, struct rico_pool *pool, FILE *fs);

struct rico_pool *object_pool_get_unsafe();
void object_pool_set_unsafe(struct rico_pool *pool);

#endif // RICO_OBJ_H