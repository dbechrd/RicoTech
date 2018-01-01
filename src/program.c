//==============================================================================
// General-purpose
//==============================================================================

internal int make_program(GLuint vertex_shader, GLuint fragment_shader,
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
        return RICO_ERROR(ERR_SHADER_LINK, "Failed to link shader %s",
                          "UNKNOWN");
    }

    *_program = program;
    return SUCCESS;
}

internal inline void free_program(GLuint program)
{
    if (program) glDeleteProgram(program);
}

internal inline GLint program_get_attrib_location(GLuint program,
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

internal inline GLint program_get_uniform_location(GLuint program,
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
// PBR program
//==============================================================================

internal inline void program_pbr_get_locations(struct program_pbr *p)
{
    // Vertex shader
    p->time = program_get_uniform_location(p->prog_id, "time");
    p->scale_uv = program_get_uniform_location(p->prog_id, "scale_uv");
    p->model = program_get_uniform_location(p->prog_id, "model");
    p->view = program_get_uniform_location(p->prog_id, "view");
    p->projection = program_get_uniform_location(p->prog_id, "projection");
    
    //RICO_ASSERT(p->u_time >= 0);
    RICO_ASSERT(p->scale_uv >= 0);
    RICO_ASSERT(p->model >= 0);
    RICO_ASSERT(p->view >= 0);
    RICO_ASSERT(p->projection >= 0);

    p->attrs.position = program_get_attrib_location(p->prog_id,"attr_position");
    p->attrs.color = program_get_attrib_location(p->prog_id, "attr_color");
    p->attrs.normal = program_get_attrib_location(p->prog_id, "attr_normal");
    p->attrs.uv = program_get_attrib_location(p->prog_id, "attr_uv");

    RICO_ASSERT(p->attrs.position == RICO_SHADER_POS_LOC);
    //RICO_ASSERT(p->attrs.color == RICO_SHADER_COL_LOC);
    RICO_ASSERT(p->attrs.normal == RICO_SHADER_NORMAL_LOC);
    RICO_ASSERT(p->attrs.uv == RICO_SHADER_UV_LOC);

    // Fragment shader
    p->camera.pos = program_get_uniform_location(p->prog_id, "camera.P");
    p->material.tex0 = program_get_uniform_location(p->prog_id, "material.tex0");
    p->material.tex1 = program_get_uniform_location(p->prog_id, "material.tex1");
    p->light.pos = program_get_uniform_location(p->prog_id, "light.P");
    p->light.color = program_get_uniform_location(p->prog_id, "light.color");
    p->light.intensity = program_get_uniform_location(p->prog_id, "light.intensity");

    RICO_ASSERT(p->camera.pos >= 0);
    RICO_ASSERT(p->material.tex0 >= 0);
    RICO_ASSERT(p->material.tex1 >= 0);
    RICO_ASSERT(p->light.pos >= 0);
    RICO_ASSERT(p->light.color >= 0);
    RICO_ASSERT(p->light.intensity >= 0);
}

int make_program_pbr(struct program_pbr **_program)
{
    local struct program_pbr *prog_pbr = NULL;
    enum rico_error err;

    if (prog_pbr != NULL) {
        *_program = prog_pbr;
        return SUCCESS;
    }

    GLuint vshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    // Compile shaders
    err = make_shader(GL_VERTEX_SHADER, "shader/pbr_v.glsl", &vshader);
    if (err) goto cleanup;

    err = make_shader(GL_FRAGMENT_SHADER, "shader/pbr_f.glsl", &fshader);
    if (err) goto cleanup;

    // Link shader program
    err = make_program(vshader, fshader, &program);
    if (err) goto cleanup;

    // Create program object
    prog_pbr = calloc(1, sizeof(*prog_pbr));
    prog_pbr->prog_id = program;

    // Query shader locations
    program_pbr_get_locations(prog_pbr);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_pbr;
    return err;
}

void free_program_pbr(struct program_pbr **program)
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

void program_pbr_uniform_projection(struct program_pbr *program,
                                    struct mat4 *proj)
{
    glUseProgram(program->prog_id);
    glUniformMatrix4fv(program->projection, 1, GL_TRUE, proj->a);
    glUseProgram(0);
}

//==============================================================================
// Default program
//==============================================================================

internal inline void program_default_get_locations(struct program_default *p)
{
    // Vertex shader
    p->u_time = program_get_uniform_location(p->prog_id, "u_time");
    p->u_scale_uv = program_get_uniform_location(p->prog_id, "u_scale_uv");
    p->u_model = program_get_uniform_location(p->prog_id, "u_model");
    p->u_view = program_get_uniform_location(p->prog_id, "u_view");
    p->u_projection = program_get_uniform_location(p->prog_id, "u_projection");

    //RICO_ASSERT(p->u_time >= 0);
    RICO_ASSERT(p->u_scale_uv >= 0);
    RICO_ASSERT(p->u_model >= 0);
    RICO_ASSERT(p->u_view >= 0);
    RICO_ASSERT(p->u_projection >= 0);

    p->u_attr.position = program_get_attrib_location(p->prog_id, "attr_position");
    p->u_attr.color = program_get_attrib_location(p->prog_id, "attr_color");
    p->u_attr.normal = program_get_attrib_location(p->prog_id, "attr_normal");
    p->u_attr.uv = program_get_attrib_location(p->prog_id, "attr_uv");

    RICO_ASSERT(p->u_attr.position == RICO_SHADER_POS_LOC);
    //RICO_ASSERT(p->u_attr.color == RICO_SHADER_COL_LOC);
    RICO_ASSERT(p->u_attr.normal == RICO_SHADER_NORMAL_LOC);
    RICO_ASSERT(p->u_attr.uv == RICO_SHADER_UV_LOC);

    // Fragment shader
    p->u_camera.position =
        program_get_uniform_location(p->prog_id, "u_camera.position");
    p->u_material.diffuse =
        program_get_uniform_location(p->prog_id, "u_material.diffuse");
    p->u_material.specular =
        program_get_uniform_location(p->prog_id, "u_material.specular");
    p->u_material.shiny =
        program_get_uniform_location(p->prog_id, "u_material.shiny");
    p->u_light_point.position =
        program_get_uniform_location(p->prog_id, "u_light_point.position");
    p->u_light_point.ambient =
        program_get_uniform_location(p->prog_id, "u_light_point.ambient");
    p->u_light_point.color =
        program_get_uniform_location(p->prog_id, "u_light_point.color");
    p->u_light_point.kc =
        program_get_uniform_location(p->prog_id, "u_light_point.kc");
    p->u_light_point.kl =
        program_get_uniform_location(p->prog_id, "u_light_point.kl");
    p->u_light_point.kq =
        program_get_uniform_location(p->prog_id, "u_light_point.kq");

    RICO_ASSERT(p->u_camera.position >= 0);
    RICO_ASSERT(p->u_material.diffuse >= 0);
    RICO_ASSERT(p->u_material.specular >= 0);
    RICO_ASSERT(p->u_material.shiny >= 0);
    RICO_ASSERT(p->u_light_point.position >= 0);
    RICO_ASSERT(p->u_light_point.ambient >= 0);
    RICO_ASSERT(p->u_light_point.color >= 0);
    RICO_ASSERT(p->u_light_point.kc >= 0);
    RICO_ASSERT(p->u_light_point.kl >= 0);
    RICO_ASSERT(p->u_light_point.kq >= 0);
}

int make_program_default(struct program_default **_program)
{
    local struct program_default *prog_default = NULL;
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
    prog_default = calloc(1, sizeof(*prog_default));
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
    glUniformMatrix4fv(program->u_projection, 1, GL_TRUE, proj->a);
    glUseProgram(0);
}

//==============================================================================
// Primitive program
//==============================================================================

internal inline void program_primitive_get_locations(struct program_primitive *p)
{
    // Vertex shader
    p->u_model = program_get_uniform_location(p->prog_id, "u_model");
    p->u_view = program_get_uniform_location(p->prog_id, "u_view");
    p->u_proj = program_get_uniform_location(p->prog_id, "u_proj");

    p->vert_pos = program_get_attrib_location(p->prog_id, "vert_pos");
    p->vert_col = program_get_attrib_location(p->prog_id, "vert_col");

    RICO_ASSERT(p->vert_pos == RICO_SHADER_POS_LOC);
    RICO_ASSERT(p->vert_col == RICO_SHADER_COL_LOC);

    // Fragment shader
    p->u_col = program_get_uniform_location(p->prog_id, "u_col");
}

int make_program_primitive(struct program_primitive **_program)
{
    local struct program_primitive *prog_primitive = NULL;
    enum rico_error err;

    if (prog_primitive != NULL) {
        *_program = prog_primitive;
        return SUCCESS;
    }

    GLuint vshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    // Compile shaders
    err = make_shader(GL_VERTEX_SHADER, "shader/prim.vert.glsl", &vshader);
    if (err) goto cleanup;

    err = make_shader(GL_FRAGMENT_SHADER, "shader/prim.frag.glsl", &fshader);
    if (err) goto cleanup;

    // Link shader program
    err = make_program(vshader, fshader, &program);
    if (err) goto cleanup;

    // Create program object
    prog_primitive = calloc(1, sizeof(*prog_primitive));
    prog_primitive->prog_id = program;

    // Query shader locations
    program_primitive_get_locations(prog_primitive);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_primitive;
    return err;
}

void free_program_primitive(struct program_primitive **program)
{
    glDeleteProgram((*program)->prog_id);
    free(*program);
    *program = NULL;
}

void program_primitive_uniform_projection(struct program_primitive *program,
                                          struct mat4 *proj)
{
    glUseProgram(program->prog_id);
    glUniformMatrix4fv(program->u_proj, 1, GL_TRUE, proj->a);
    glUseProgram(0);
}