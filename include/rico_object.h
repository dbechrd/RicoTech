#ifndef RICO_OBJ_H
#define RICO_OBJ_H

#include "geom.h"
#include "rico_mesh.h"
#include "rico_uid.h"

struct program_default;

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

//Cleanup: Dunno if there are needed outside of this CU
//extern struct rico_object object_pool[];
//extern u32 next_uid;

int object_init(u32 pool_size);
int object_create(u32 *_handle, const char *name, enum rico_object_type type,
                  u32 mesh, u32 texture, const struct bbox *bbox);
void object_free(u32 handle);
void object_free_all();
struct rico_object *object_fetch(u32 handle);
u32 object_next(u32 handle);
u32 object_prev(u32 handle);
void object_select(u32 handle);
void object_deselect(u32 handle);
void object_trans(u32 handle, float x, float y, float z);
void object_rot_x(u32 handle, float deg);
void object_rot_y(u32 handle, float deg);
void object_rot_z(u32 handle, float deg);
void object_scale(u32 handle, float x, float y, float z);
void object_render(u32 handle, const struct program_default *prog,
                   const struct camera *camera);
void object_render_type(enum rico_object_type type,
                        const struct program_default *prog,
                        const struct camera *camera);
int object_serialize_0(const void *handle, const struct rico_file *file);
int object_deserialize_0(void *_handle, const struct rico_file *file);

struct rico_pool *object_pool_get_unsafe();
void object_pool_set_unsafe(struct rico_pool *pool);

#endif // RICO_OBJ_H