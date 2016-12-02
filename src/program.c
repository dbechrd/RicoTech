#include "program.h"
#include "const.h"
#include "geom.h"
#include "shader.h"

#include <stdlib.h>
#include <stdio.h>

//==============================================================================
// General-purpose
//==============================================================================

static int make_program(GLuint vertex_shader, GLuint fragment_shader,
                        GLuint *_program)
{
    GLint status;
    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);

        //Clean up
        glDeleteProgram(program);
        return RICO_ERROR(ERR_SHADER_LINK);
    }

    *_program = program;
    return SUCCESS;
}

static inline void free_program(GLuint program)
{
    if (program) glDeleteProgram(program);
}

static inline GLint program_get_attrib_location(GLuint program,
                                                const char* name)
{
    GLint location = glGetAttribLocation(program, name);
    if (location < 0)
    {
        fprintf(stderr, "[Program %d] Location not found for attribute '%s'. "
                "Possibly optimized out.\n", program, name);
    }

    return location;
}

static inline GLint program_get_uniform_location(GLuint program,
                                                 const char* name)
{
    GLint location = glGetUniformLocation(program, name);
    if (location < 0)
    {
        fprintf(stderr, "[Program %d] Location not found for attribute '%s'. "
                "Possibly optimized out.\n", program, name);
    }

    return location;
}

//==============================================================================
// Default program
//==============================================================================

static inline void program_default_get_locations(struct program_default *p)
{
    // Vertex shader
    //p->u_time = program_get_uniform_location(p->prog_id, "u_time");
    p->u_scale_uv = program_get_uniform_location(p->prog_id, "u_scale_uv");
    p->u_model = program_get_uniform_location(p->prog_id, "u_model");
    p->u_view = program_get_uniform_location(p->prog_id, "u_view");
    p->u_proj = program_get_uniform_location(p->prog_id, "u_proj");

    p->vert_pos = program_get_attrib_location(p->prog_id, "vert_pos");
    //p->vert_col = program_get_attrib_location(p->prog_id, "vert_col");
    p->vert_uv = program_get_attrib_location(p->prog_id, "vert_uv");

    // Fragment shader
    //p->u_ambient = program_get_uniform_location(p->prog_id, "u_ambient");
    p->u_tex = program_get_uniform_location(p->prog_id, "u_tex");
}

int make_program_default(struct program_default **_program)
{
    static struct program_default *prog_default = NULL;
    enum rico_error err;

    if (prog_default != NULL) {
        *_program = prog_default;
        return SUCCESS;
    }

    GLuint vshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    // Compile shaders
    err = make_shader(GL_VERTEX_SHADER, "shader/default.vert.glsl", &vshader);
    if (err) goto cleanup;

    err = make_shader(GL_FRAGMENT_SHADER, "shader/default.frag.glsl", &fshader);
    if (err) goto cleanup;

    // Link shader program
    err = make_program(vshader, fshader, &program);
    if (err) goto cleanup;

    // Create program object
    prog_default = calloc(1, sizeof(struct program_default));
    prog_default->prog_id = program;

    // Query shader locations
    program_default_get_locations(prog_default);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_default;
    return err;
}

void free_program_default(struct program_default **program)
{
    //TODO: Handle error
    if ((*program)->ref_count > 0) {
        printf("Cannot delete a program in use!");
        //TODO: crash;
    }

    glDeleteProgram((*program)->prog_id);
    free(*program);
    *program = NULL;
}

void program_default_uniform_projection(struct program_default *program,
                                        struct mat4 *proj)
{
    glUseProgram(program->prog_id);
    glUniformMatrix4fv(program->u_proj, 1, GL_TRUE, proj->a);
    glUseProgram(0);
}

//==============================================================================
// BBox program
//==============================================================================

static inline void program_bbox_get_locations(struct program_bbox *p)
{
    // Vertex shader
    p->u_model = program_get_uniform_location(p->prog_id, "u_model");
    p->u_view = program_get_uniform_location(p->prog_id, "u_view");
    p->u_proj = program_get_uniform_location(p->prog_id, "u_proj");
    p->u_color = program_get_uniform_location(p->prog_id, "u_color");

    p->vert_pos = program_get_attrib_location(p->prog_id, "vert_pos");

    // Fragment shader
    //p->u_ambient = program_get_uniform_location(p->prog_id, "u_ambient");
}

int make_program_bbox(struct program_bbox **_program)
{
    static struct program_bbox *prog_bbox = NULL;
    enum rico_error err;

    if (prog_bbox != NULL) {
        *_program = prog_bbox;
        return SUCCESS;
    }

    GLuint vshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    // Compile shaders
    err = make_shader(GL_VERTEX_SHADER, "shader/bbox.vert.glsl", &vshader);
    if (err) goto cleanup;

    err = make_shader(GL_FRAGMENT_SHADER, "shader/bbox.frag.glsl", &fshader);
    if (err) goto cleanup;

    // Link shader program
    err = make_program(vshader, fshader, &program);
    if (err) goto cleanup;

    // Create program object
    prog_bbox = calloc(1, sizeof(struct program_bbox));
    prog_bbox->prog_id = program;

    // Query shader locations
    program_bbox_get_locations(prog_bbox);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_bbox;
    return err;
}

void free_program_bbox(struct program_bbox **program)
{
    //TODO: Handle error
    if ((*program)->ref_count > 0) {
        printf("Cannot delete a program in use!");
        RICO_ASSERT(0);
    }

    glDeleteProgram((*program)->prog_id);
    free(*program);
    *program = NULL;
}

void program_bbox_uniform_projection(struct program_bbox *program,
                                     struct mat4 *proj)
{
    glUseProgram(program->prog_id);
    glUniformMatrix4fv(program->u_proj, 1, GL_TRUE, proj->a);
    glUseProgram(0);
}