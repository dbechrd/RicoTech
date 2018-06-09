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
static void program_pbr_get_locations(struct pbr_program *p)
{
    // Vertex shader
    p->vert.scale_uv = program_get_uniform_location(p->program.gl_id, "scale_uv");
    p->vert.proj = program_get_uniform_location(p->program.gl_id, "proj");
    p->vert.view = program_get_uniform_location(p->program.gl_id, "view");
    p->vert.model = program_get_uniform_location(p->program.gl_id, "model");
    RICO_ASSERT(p->vert.scale_uv >= 0);
    RICO_ASSERT(p->vert.model >= 0);
    RICO_ASSERT(p->vert.view >= 0);
    RICO_ASSERT(p->vert.proj >= 0);

    p->vert.attrs.position = program_get_attrib_location(p->program.gl_id,"attr_position");
    p->vert.attrs.uv = program_get_attrib_location(p->program.gl_id, "attr_uv");
    p->vert.attrs.color = program_get_attrib_location(p->program.gl_id, "attr_color");
    p->vert.attrs.normal = program_get_attrib_location(p->program.gl_id, "attr_normal");
    RICO_ASSERT(p->vert.attrs.position == LOCATION_PBR_POSITION);
    RICO_ASSERT(p->vert.attrs.uv == LOCATION_PBR_UV);
    //RICO_ASSERT(p->vert.attrs.color == LOCATION_PBR_COLOR);
    RICO_ASSERT(p->vert.attrs.normal == LOCATION_PBR_NORMAL);

    // Fragment shader
    p->frag.camera.pos = program_get_uniform_location(p->program.gl_id, "camera.P");
    p->frag.material.tex0 = program_get_uniform_location(p->program.gl_id, "material.tex0");
    p->frag.material.tex1 = program_get_uniform_location(p->program.gl_id, "material.tex1");
    p->frag.material.tex2 = program_get_uniform_location(p->program.gl_id, "material.tex2");
    p->frag.light.pos = program_get_uniform_location(p->program.gl_id, "light.P");
    p->frag.light.color = program_get_uniform_location(p->program.gl_id, "light.color");
    p->frag.light.intensity = program_get_uniform_location(p->program.gl_id, "light.intensity");
    RICO_ASSERT(p->frag.camera.pos >= 0);
    RICO_ASSERT(p->frag.material.tex0 >= 0);
    RICO_ASSERT(p->frag.material.tex1 >= 0);
    RICO_ASSERT(p->frag.material.tex2 >= 0);
    RICO_ASSERT(p->frag.light.pos >= 0);
    RICO_ASSERT(p->frag.light.color >= 0);
    RICO_ASSERT(p->frag.light.intensity >= 0);
}
static void program_pbr_attribs()
{
    glVertexAttribPointer(LOCATION_PBR_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, pos));
    glEnableVertexAttribArray(LOCATION_PBR_POSITION);

    glVertexAttribPointer(LOCATION_PBR_UV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, uv));
    glEnableVertexAttribArray(LOCATION_PBR_UV);

    glVertexAttribPointer(LOCATION_PBR_COLOR, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, col));
    glEnableVertexAttribArray(LOCATION_PBR_COLOR);

    glVertexAttribPointer(LOCATION_PBR_NORMAL, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, normal));
    glEnableVertexAttribArray(LOCATION_PBR_NORMAL);
}
static int make_program_pbr(struct pbr_program **_program)
{
    static struct pbr_program *prog_pbr = NULL;
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
    prog_pbr->program.type = PROG_PBR;
    prog_pbr->program.gl_id = program;

    // Query shader locations
    program_pbr_get_locations(prog_pbr);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_pbr;
    return err;
}
static void free_program_pbr(struct pbr_program **program)
{
    //TODO: Handle error
    if ((*program)->program.ref_count > 0) {
        printf("Cannot delete a program in use!");
        //TODO: crash;
    }

    glDeleteProgram((*program)->program.gl_id);
    free(*program);
    *program = NULL;
}

///=============================================================================
//| Primitive program
///=============================================================================
static void program_primitive_get_locations(struct prim_program *p)
{
    // Vertex shader
    p->vert.proj = program_get_uniform_location(p->program.gl_id, "proj");
    p->vert.view = program_get_uniform_location(p->program.gl_id, "view");
    p->vert.model = program_get_uniform_location(p->program.gl_id, "model");
    RICO_ASSERT(p->vert.proj >= 0);
    RICO_ASSERT(p->vert.view >= 0);
    RICO_ASSERT(p->vert.model >= 0);

    p->vert.attrs.position = program_get_attrib_location(p->program.gl_id, "attr_position");
    p->vert.attrs.uv =  program_get_attrib_location(p->program.gl_id, "attr_uv");
    p->vert.attrs.color = program_get_attrib_location(p->program.gl_id, "attr_color");
    RICO_ASSERT(p->vert.attrs.position == LOCATION_PBR_POSITION);
    //RICO_ASSERT(p->vert.attrs.uv == LOCATION_PBR_UV);
    //RICO_ASSERT(p->vert.attrs.color == LOCATION_PBR_COLOR);

    // Fragment shader
    p->frag.tex = program_get_uniform_location(p->program.gl_id, "tex");
    //RICO_ASSERT(p->frag.tex >= 0);
}
static void program_primitive_attribs()
{
    glVertexAttribPointer(LOCATION_PRIM_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, pos));
    glEnableVertexAttribArray(LOCATION_PRIM_POSITION);

    glVertexAttribPointer(LOCATION_PRIM_UV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, uv));
    glEnableVertexAttribArray(LOCATION_PRIM_UV);

    glVertexAttribPointer(LOCATION_PRIM_COLOR, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, col));
    glEnableVertexAttribArray(LOCATION_PRIM_COLOR);
}
static int make_program_primitive(struct prim_program **_program)
{
    static struct prim_program *prog_primitive = NULL;
    enum RICO_error err;

    if (prog_primitive != NULL) {
        *_program = prog_primitive;
        return SUCCESS;
    }

    GLuint vshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    // Compile shaders
    err = make_shader(GL_VERTEX_SHADER, "shader/prim_v.glsl", &vshader);
    if (err) goto cleanup;

    err = make_shader(GL_FRAGMENT_SHADER, "shader/prim_f.glsl", &fshader);
    if (err) goto cleanup;

    // Link shader program
    err = make_program(vshader, fshader, &program);
    if (err) goto cleanup;

    // Create program object
    prog_primitive = calloc(1, sizeof(*prog_primitive));
    prog_primitive->program.type = PROG_PRIM;
    prog_primitive->program.gl_id = program;

    // Query shader locations
    program_primitive_get_locations(prog_primitive);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_primitive;
    return err;
}
static void free_program_primitive(struct prim_program **program)
{
    glDeleteProgram((*program)->program.gl_id);
    free(*program);
    *program = NULL;
}

///=============================================================================
//| Text program
///=============================================================================
static void program_text_get_locations(struct text_program *p)
{
    // Vertex shader
    p->vert.model = program_get_uniform_location(p->program.gl_id, "model");
    p->vert.proj = program_get_uniform_location(p->program.gl_id, "proj");

    RICO_ASSERT(p->vert.model >= 0);
    RICO_ASSERT(p->vert.proj >= 0);

    // Fragment shader
    p->frag.tex = program_get_uniform_location(p->program.gl_id, "tex");

    RICO_ASSERT(p->frag.tex >= 0);
}
static void program_text_attribs()
{
    glVertexAttribPointer(LOCATION_TEXT_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct text_vertex),
                          (GLvoid *)offsetof(struct text_vertex, pos));
    glEnableVertexAttribArray(LOCATION_TEXT_POSITION);

    glVertexAttribPointer(LOCATION_TEXT_UV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct text_vertex),
                          (GLvoid *)offsetof(struct text_vertex, uv));
    glEnableVertexAttribArray(LOCATION_TEXT_UV);

    glVertexAttribPointer(LOCATION_TEXT_COLOR, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct text_vertex),
                          (GLvoid *)offsetof(struct text_vertex, col));
    glEnableVertexAttribArray(LOCATION_TEXT_COLOR);
}
static int make_program_text(struct text_program **_program)
{
    static struct text_program *prog_text = NULL;
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
    prog_text->program.type = PROG_TEXT;
    prog_text->program.gl_id = program;

    // Query shader locations
    program_text_get_locations(prog_text);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog_text;
    return err;
}
static void free_program_text(struct text_program **program)
{
    glDeleteProgram((*program)->program.gl_id);
    free(*program);
    *program = NULL;
}
