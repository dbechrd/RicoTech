#ifndef RICO_OBJ_H
#define RICO_OBJ_H

#include "const.h"
#include "geom.h"
#include "mesh.h"
#include <GL/gl3w.h>

struct rico_obj {
    //TODO: Should this be a rico_uid with a string name?
    uint32 uid;

    //TODO: Refactor into rico_transform
    //TODO: Animation
    struct vec4 trans;
    struct mat4 rot;
    struct vec4 scale;

    //TODO: Support multiple meshes
    struct rico_mesh mesh;
};

#endif // RICO_OBJ_H