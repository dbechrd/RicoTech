#ifndef PROGRAM_H
#define PROGRAM_H

#include "const.h"
#include "shader.h"
#include "geom.h"
#include <GL/gl3w.h>

//--------------------------------------------------------------------------
// Default program
//--------------------------------------------------------------------------
struct program_default {
    u32 ref_count;
    GLuint prog_id;

    //TODO: Don't set from outside, create wrapper methods to enforce type
    // Vertex shader
    GLint u_time;     // float
    GLint u_scale_uv; // vec3
    GLint u_model;    // mat4
    GLint u_view;     // mat4
    GLint u_proj;     // mat4

    GLint vert_pos;    // vec3
    GLint vert_normal; // vec3
    GLint vert_col;    // vec3
    GLint vert_uv;     // vec2

    // Fragment shader
    GLint u_view_pos; // vec3

    GLint u_material_diff;  // sampler2D
    GLint u_material_spec;  // sampler2D
    GLint u_material_shiny; // float

    GLint u_light_position; // vec3
    GLint u_light_ambient;  // vec3
    GLint u_light_color;    // vec3
    GLint u_light_kc;       // float
    GLint u_light_kl;       // float
    GLint u_light_kq;       // float
};

int make_program_default(struct program_default **_program);
void free_program_default(struct program_default **program);
void program_default_uniform_projection(struct program_default *program,
                                        struct mat4 *proj);

//--------------------------------------------------------------------------
// Primitive program
//--------------------------------------------------------------------------
struct program_primitive {
    GLuint prog_id;

    // Vertex shader
    GLint u_model;  //mat4
    GLint u_view;   //mat4
    GLint u_proj;   //mat4

    GLint vert_pos; //vec3
    GLint vert_col; //vec3

    // Fragment shader
    GLint u_col;    //vec3
};

int make_program_primitive(struct program_primitive **_program);
void free_program_primitive(struct program_primitive **program);
void program_primitive_uniform_projection(struct program_primitive *program,
                                          struct mat4 *proj);

#endif // PROGRAM_H
