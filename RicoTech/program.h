#ifndef PROGRAM_H
#define PROGRAM_H

#include "const.h"
#include "shader.h"
#include <GL/gl3w.h>

//--------------------------------------------------------------------------
// Default program
//--------------------------------------------------------------------------

struct program_default {
    uint32 ref_count;
    GLuint prog_id;

    //TODO: Don't set from outside, create wrapper methods to enforce type
    GLint u_time;     //float
    GLint u_scale_uv; //vec4
    GLint u_model;    //mat4
    GLint u_view;     //mat4
    GLint u_proj;     //mat4

    GLint u_ambient;  //vec4
    GLint u_tex;      //sampler2D

    GLint vert_pos;   //vec4
    GLint vert_col;   //vec4
    GLint vert_uv;    //vec2
};

struct program_default *make_program_default();
void free_program_default(struct program_default **program);
void program_default_uniform_projection(struct program_default *program,
                                        struct mat4 *proj);

//--------------------------------------------------------------------------
// BBox program
//--------------------------------------------------------------------------

struct program_bbox {
    uint32 ref_count;
    GLuint prog_id;

    //TODO: Don't set from outside, create wrapper methods to enforce type
    GLint u_model;  //mat4
    GLint u_view;   //mat4
    GLint u_proj;   //mat4
    GLint u_color;  //vec4

    GLint vert_pos; //vec4
};

struct program_bbox *make_program_bbox();
void free_program_bbox(struct program_bbox **program);
void program_bbox_uniform_projection(struct program_bbox *program,
                                     struct mat4 *proj);
#endif // PROGRAM_H
