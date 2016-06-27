#include "program.h"
#include "const.h"

#include <stdlib.h>
#include <stdio.h>

//==============================================================================
// General-purpose
//==============================================================================

static inline GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint status;
    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);

        //Clean up
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

static inline void free_program(GLuint program)
{
    glDeleteProgram(program);
}

static inline GLint program_get_attrib_location(GLuint program,
                                                const char* name)
{
    GLint location = glGetAttribLocation(program, name);
    if (location == LOCATION_NULL)
    {
        fprintf(stderr, "[Program] Location not found for attribute '%s'. "
                "Possibly optimized out.\n", name);
    }

    return location;
}

static inline GLint program_get_uniform_location(GLuint program,
                                                 const char* name)
{
    GLint location = glGetUniformLocation(program, name);
    if (location == LOCATION_NULL)
    {
        fprintf(stderr, "[Program] Location not found for attribute '%s'. "
                "Possibly optimized out.\n", name);
    }

    return location;
}

//==============================================================================
// Default program
//==============================================================================

static inline void program_default_get_locations(struct program_default *p)
{
    // Get uniform locations
    (*p).u_time = program_get_uniform_location((*p).program, "u_time");
    (*p).u_scale_uv = program_get_uniform_location((*p).program, "u_scale_uv");
    (*p).u_model = program_get_uniform_location((*p).program, "u_model");
    (*p).u_view = program_get_uniform_location((*p).program, "u_view");
    (*p).u_proj = program_get_uniform_location((*p).program, "u_proj");
    (*p).u_tex = program_get_uniform_location((*p).program, "u_tex");
    
    // Get vertex attribute locations
    (*p).vert_pos = program_get_attrib_location((*p).program, "vert_pos");
    (*p).vert_col = program_get_attrib_location((*p).program, "vert_col");
    (*p).vert_uv = program_get_attrib_location((*p).program, "vert_uv");
}

struct program_default *make_program_default()
{
    struct program_default *prog_default = NULL;

    // Compile shaders
    GLuint vshader = make_shader(GL_VERTEX_SHADER, "default.vert.glsl");
    if (!vshader) goto err_vert;

    GLuint fshader = make_shader(GL_FRAGMENT_SHADER, "default.frag.glsl");
    if (!fshader) goto err_frag;
    
    // Link shader program
    GLuint program = make_program(vshader, fshader);
    if (!program) goto err_prog;

    // Create default program object
    prog_default =
        (struct program_default *)malloc(sizeof(struct program_default));

    prog_default->program = program;
    program_default_get_locations(prog_default);

    vshader = 0;

err_prog:
    free_shader(fshader);
err_frag:
    free_shader(vshader);
err_vert:
    return prog_default;
}

void free_program_default(struct program_default **program)
{
    glDeleteProgram((*program)->program);
    free(*program);
    *program = NULL;

    // No need to zero memory if we're storing program as a pointer
    //(*program).program = PROGRAM_NULL;
    //
    //(*program).uniform_time = LOCATION_NULL;
    //(*program).uniform_model = LOCATION_NULL;
    //(*program).uniform_view = LOCATION_NULL;
    //(*program).uniform_proj = LOCATION_NULL;
    //(*program).uniform_tex = LOCATION_NULL;
    //
    //(*program).vertex_pos = LOCATION_NULL;
    //(*program).vertex_col = LOCATION_NULL;
    //(*program).vertex_uv = LOCATION_NULL;
}