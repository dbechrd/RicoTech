#ifndef PROGRAM_H
#define PROGRAM_H

#include "shader.h"
#include <GL/gl3w.h>

struct program_default {
    GLuint program;

    GLint uniform_time;  //float
    GLint uniform_model; //mat4
    GLint uniform_view;  //mat4
    GLint uniform_proj;  //mat4
    GLint uniform_tex;   //sampler2D
                         
    GLint vertex_pos;    //vec4
    GLint vertex_col;    //vec4
    GLint vertex_uv;     //vec2
};

static inline GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint status;
    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

static inline struct program_default program_default_locations(GLuint program)
{
    struct program_default loc;

    loc.program = program;

    loc.uniform_time = glGetUniformLocation(program, "u_time");
    loc.uniform_model = glGetUniformLocation(program, "u_model");
    loc.uniform_view = glGetUniformLocation(program, "u_view");
    loc.uniform_proj = glGetUniformLocation(program, "u_proj");
    loc.uniform_tex = glGetUniformLocation(program, "u_tex");

    loc.vertex_pos = glGetAttribLocation(program, "vert_pos");
    loc.vertex_col = glGetAttribLocation(program, "vert_col");
    loc.vertex_uv = glGetAttribLocation(program, "vert_uv");

    return loc;
}

#endif // PROGRAM_H
