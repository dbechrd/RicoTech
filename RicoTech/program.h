#ifndef PROGRAM_H
#define PROGRAM_H

#include "shader.h"
#include <GL/gl3w.h>

//--------------------------------------------------------------------------
// Default program
//--------------------------------------------------------------------------

struct program_default {
    GLuint prog_id;

    //TODO?: Don't set from outside, create wrapper methods to enforce type
    GLint u_time;     //float
    GLint u_scale_uv; //vec4
    GLint u_model;    //mat4
    GLint u_view;     //mat4
    GLint u_proj;     //mat4
    GLint u_tex;      //sampler2D

    GLint vert_pos;   //vec4
    GLint vert_col;   //vec4
    GLint vert_uv;    //vec2
};

struct program_default *make_program_default();
void free_program_default(struct program_default **program);

//--------------------------------------------------------------------------
// BBox program
//--------------------------------------------------------------------------

struct program_bbox {
    GLuint prog_id;

    GLint u_model;  //mat4
    GLint u_view;   //mat4
    GLint u_proj;   //mat4
    GLint u_color;  //vec4

    GLint vert_pos; //vec4
};

struct program_bbox *make_program_bbox();
void free_program_bbox(struct program_bbox **program);

#endif // PROGRAM_H
