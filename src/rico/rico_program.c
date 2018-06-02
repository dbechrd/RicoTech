static program_attribs_helper program_attribs[PROG_COUNT] = {
    0,
    program_pbr_attribs,
    program_primitive_attribs,
    program_text_attribs
};

///=============================================================================
//| General-purpose
///=============================================================================
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
        return RICO_ERROR(ERR_SHADER_LINK, "Failed to link shader %s",
                          "UNKNOWN");
    }

    *_program = program;
    return SUCCESS;
}
static void free_program(GLuint program)
{
    if (program) glDeleteProgram(program);
}
static GLint program_get_attrib_location(GLuint program,
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
static GLint program_get_uniform_location(GLuint program,
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

///=============================================================================
//| PBR program
///=============================================================================
static void program_pbr_get_locations(struct program_pbr *p)
{
    // Vertex shader
    p->time = program_get_uniform_location(p->prog_id, "time");
    p->scale_uv = program_get_uniform_location(p->prog_id, "scale_uv");
    p->model = program_get_uniform_location(p->prog_id, "model");
    p->view = program_get_uniform_location(p->prog_id, "view");
    p->proj = program_get_uniform_location(p->prog_id, "proj");

    //RICO_ASSERT(p->u_time >= 0);
    RICO_ASSERT(p->scale_uv >= 0);
    RICO_ASSERT(p->model >= 0);
    RICO_ASSERT(p->view >= 0);
    RICO_ASSERT(p->proj >= 0);

    p->attrs.position = program_get_attrib_location(p->prog_id,"attr_position");
    p->attrs.color = program_get_attrib_location(p->prog_id, "attr_color");
    p->attrs.normal = program_get_attrib_location(p->prog_id, "attr_normal");
    p->attrs.uv = program_get_attrib_location(p->prog_id, "attr_uv");

    RICO_ASSERT(p->attrs.position == LOCATION_PBR_POSITION);
    //RICO_ASSERT(p->attrs.color == RICO_SHADER_COL_LOC);
    RICO_ASSERT(p->attrs.normal == LOCATION_PBR_NORMAL);
    RICO_ASSERT(p->attrs.uv == LOCATION_PBR_UV);

    // Fragment shader
    p->camera.pos = program_get_uniform_location(p->prog_id, "camera.P");
    p->material.tex0 = program_get_uniform_location(p->prog_id, "material.tex0");
    p->material.tex1 = program_get_uniform_location(p->prog_id, "material.tex1");
    p->material.tex2 = program_get_uniform_location(p->prog_id, "material.tex2");
    p->light.pos = program_get_uniform_location(p->prog_id, "light.P");
    p->light.color = program_get_uniform_location(p->prog_id, "light.color");
    p->light.intensity = program_get_uniform_location(p->prog_id, "light.intensity");

    RICO_ASSERT(p->camera.pos >= 0);
    RICO_ASSERT(p->material.tex0 >= 0);
    RICO_ASSERT(p->material.tex1 >= 0);
    RICO_ASSERT(p->material.tex2 >= 0);
    RICO_ASSERT(p->light.pos >= 0);
    RICO_ASSERT(p->light.color >= 0);
    RICO_ASSERT(p->light.intensity >= 0);
}
static void program_pbr_attribs()
{
    glVertexAttribPointer(LOCATION_PBR_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, pos));
    glEnableVertexAttribArray(LOCATION_PBR_POSITION);

    glVertexAttribPointer(LOCATION_PBR_NORMAL, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, normal));
    glEnableVertexAttribArray(LOCATION_PBR_NORMAL);

    glVertexAttribPointer(LOCATION_PBR_COLOR, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, col));
    glEnableVertexAttribArray(LOCATION_PBR_COLOR);

    glVertexAttribPointer(LOCATION_PBR_UV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, uv));
    glEnableVertexAttribArray(LOCATION_PBR_UV);
}
static int make_program_pbr(struct program_pbr **_program)
{
    static struct program_pbr *prog_pbr = NULL;
    enum RICO_error err;

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
    prog_pbr->type = PROG_PBR;
    prog_pbr->prog_id = program;

    // Query shader locations
    program_pbr_get_locations(prog_pbr);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_pbr;
    return err;
}
static void free_program_pbr(struct program_pbr **program)
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

///=============================================================================
//| Primitive program
///=============================================================================
static void program_primitive_get_locations(struct program_primitive *p)
{
    // Vertex shader
    p->u_model = program_get_uniform_location(p->prog_id, "u_model");
    p->u_view = program_get_uniform_location(p->prog_id, "u_view");
    p->u_proj = program_get_uniform_location(p->prog_id, "u_proj");

    p->attrs.position = program_get_attrib_location(p->prog_id, "vert_pos");

    RICO_ASSERT(p->attrs.position == LOCATION_PRIM_POSITION);

    // Fragment shader
    p->u_col = program_get_uniform_location(p->prog_id, "u_col");
}
static void program_primitive_attribs()
{
    // TODO: This should have it's own vertex type.. not even sure if this works
    glVertexAttribPointer(LOCATION_PRIM_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, pos));
    glEnableVertexAttribArray(LOCATION_PRIM_POSITION);
}
static int make_program_primitive(struct program_primitive **_program)
{
    static struct program_primitive *prog_primitive = NULL;
    enum RICO_error err;

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
    prog_primitive->type = PROG_PRIM;
    prog_primitive->prog_id = program;

    // Query shader locations
    program_primitive_get_locations(prog_primitive);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_primitive;
    return err;
}
static void free_program_primitive(struct program_primitive **program)
{
    glDeleteProgram((*program)->prog_id);
    free(*program);
    *program = NULL;
}

///=============================================================================
//| Primitive cube program
///=============================================================================
static void program_prim_cube_get_locations(struct program_prim_cube *p)
{
    // Vertex shader
    p->model = program_get_uniform_location(p->prog_id, "model");
    p->view = program_get_uniform_location(p->prog_id, "view");
    p->proj = program_get_uniform_location(p->prog_id, "proj");

    p->p0 = program_get_attrib_location(p->prog_id, "p0");
    p->p1 = program_get_attrib_location(p->prog_id, "p1");

    // Fragment shader
    p->color = program_get_uniform_location(p->prog_id, "color");
}
static int make_program_prim_cube(struct program_prim_cube **_program)
{
    static struct program_prim_cube *prog_prim_cube = NULL;
    enum RICO_error err;

    if (prog_prim_cube != NULL) {
        *_program = prog_prim_cube;
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
    prog_prim_cube = calloc(1, sizeof(*prog_prim_cube));
    prog_prim_cube->type = PROG_PRIM;
    prog_prim_cube->prog_id = program;

    // Query shader locations
    program_prim_cube_get_locations(prog_prim_cube);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_prim_cube;
    return err;
}
static void free_program_prim_cube(struct program_prim_cube **program)
{
    glDeleteProgram((*program)->prog_id);
    free(*program);
    *program = NULL;
}

///=============================================================================
//| Text program
///=============================================================================
static void program_text_get_locations(struct program_text *p)
{
    // Vertex shader
    p->model = program_get_uniform_location(p->prog_id, "model");
    p->proj = program_get_uniform_location(p->prog_id, "proj");

    RICO_ASSERT(p->model >= 0);
    RICO_ASSERT(p->proj >= 0);

    // Fragment shader
    p->tex = program_get_uniform_location(p->prog_id, "tex");

    RICO_ASSERT(p->tex >= 0);
}
static void program_text_attribs()
{
    glVertexAttribPointer(LOCATION_TEXT_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct text_vertex),
                          (GLvoid *)offsetof(struct text_vertex, pos));
    glEnableVertexAttribArray(LOCATION_TEXT_POSITION);

    glVertexAttribPointer(LOCATION_TEXT_COLOR, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct text_vertex),
                          (GLvoid *)offsetof(struct text_vertex, col));
    glEnableVertexAttribArray(LOCATION_TEXT_COLOR);

    glVertexAttribPointer(LOCATION_TEXT_UV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct text_vertex),
                          (GLvoid *)offsetof(struct text_vertex, uv));
    glEnableVertexAttribArray(LOCATION_TEXT_UV);
}
static int make_program_text(struct program_text **_program)
{
    static struct program_text *prog_text = NULL;
    enum RICO_error err;

    if (prog_text != NULL) {
        *_program = prog_text;
        return SUCCESS;
    }

    GLuint vshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    // Compile shaders
    err = make_shader(GL_VERTEX_SHADER, "shader/text_v.glsl", &vshader);
    if (err) goto cleanup;

    err = make_shader(GL_FRAGMENT_SHADER, "shader/text_f.glsl", &fshader);
    if (err) goto cleanup;

    // Link shader program
    err = make_program(vshader, fshader, &program);
    if (err) goto cleanup;

    // Create program object
    prog_text = calloc(1, sizeof(*prog_text));
    prog_text->type = PROG_TEXT;
    prog_text->prog_id = program;

    // Query shader locations
    program_text_get_locations(prog_text);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_text;
    return err;
}
static void free_program_text(struct program_text **program)
{
    glDeleteProgram((*program)->prog_id);
    free(*program);
    *program = NULL;
}
