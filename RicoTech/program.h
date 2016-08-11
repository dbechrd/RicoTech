#ifndef PROGRAM_H
#define PROGRAM_H

#include "shader.h"
#include <GL/gl3w.h>

//--------------------------------------------------------------------------
// General-purpose
//--------------------------------------------------------------------------

//Cleanup: I don't think there's any reason to make these globally accessible

//static inline GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);
//static inline void free_program(GLuint program);
//
//static inline GLint program_get_attrib_location(GLuint program,
//                                                const char* name);
//static inline GLint program_get_uniform_location(GLuint program,
//                                                 const char* name);

//--------------------------------------------------------------------------
// Default program
//--------------------------------------------------------------------------

struct program_default {
    GLuint prog_id;

    //TODO: Don't set these from outside, create wrapper methods to enforce type
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

#endif // PROGRAM_H
