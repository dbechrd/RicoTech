static program_attribs_helper program_attribs[PROG_COUNT] =
{
    0,
    program_pbr_attribs,
    program_shadow_attribs,
    program_primitive_attribs,
    program_text_attribs
};

///=============================================================================
//| General-purpose
///=============================================================================
static int make_program(GLuint vertex_shader, GLuint geometry_shader,
                        GLuint fragment_shader, GLuint *_program)
{
    GLint status;
    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    if (geometry_shader)
        glAttachShader(program, geometry_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDetachShader(program, vertex_shader);
    if (geometry_shader)
        glDetachShader(program, geometry_shader);
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
static GLint program_get_attrib_location(GLuint program, const char* name)
{
    GLint location = glGetAttribLocation(program, name);
    if (location < 0)
    {
        fprintf(stderr, "[Program %d] Location not found for attribute '%s'. "
                "Possibly optimized out.\n", program, name);
    }

    return location;
}
static GLint program_get_uniform_location(GLuint program, const char* name)
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
    p->locations.vert.scale_uv =
        program_get_uniform_location(p->program.gl_id, "scale_uv");
    p->locations.vert.proj =
        program_get_uniform_location(p->program.gl_id, "proj");
    p->locations.vert.view =
        program_get_uniform_location(p->program.gl_id, "view");
    p->locations.vert.model =
        program_get_uniform_location(p->program.gl_id, "model");
    RICO_ASSERT(p->locations.vert.scale_uv >= 0);
    RICO_ASSERT(p->locations.vert.model >= 0);
    RICO_ASSERT(p->locations.vert.view >= 0);
    RICO_ASSERT(p->locations.vert.proj >= 0);

    p->locations.vert.attrs.position =
        program_get_attrib_location(p->program.gl_id,"attr_position");
    p->locations.vert.attrs.uv =
        program_get_attrib_location(p->program.gl_id, "attr_uv");
    p->locations.vert.attrs.color =
        program_get_attrib_location(p->program.gl_id, "attr_color");
    p->locations.vert.attrs.normal =
        program_get_attrib_location(p->program.gl_id, "attr_normal");
    RICO_ASSERT(p->locations.vert.attrs.position == LOCATION_PBR_POSITION);
    RICO_ASSERT(p->locations.vert.attrs.uv == LOCATION_PBR_UV);
    // TODO: Turn these back on when they're being used
    //RICO_ASSERT(p->locations.vert.attrs.color == LOCATION_PBR_COLOR);
    //RICO_ASSERT(p->locations.vert.attrs.normal == LOCATION_PBR_NORMAL);

    // Fragment shader
    p->locations.frag.camera.pos =
        program_get_uniform_location(p->program.gl_id, "camera.P");
    p->locations.frag.material.tex0 =
        program_get_uniform_location(p->program.gl_id, "material.tex0");
    p->locations.frag.material.tex1 =
        program_get_uniform_location(p->program.gl_id, "material.tex1");
    p->locations.frag.material.tex2 =
        program_get_uniform_location(p->program.gl_id, "material.tex2");

#define POINT_LIGHT(i)                                                        \
p->locations.frag.lights[i].pos =                                             \
    program_get_uniform_location(p->program.gl_id, "lights["#i"].P");         \
p->locations.frag.lights[i].color =                                           \
    program_get_uniform_location(p->program.gl_id, "lights["#i"].color");     \
p->locations.frag.lights[i].intensity =                                       \
    program_get_uniform_location(p->program.gl_id, "lights["#i"].intensity"); \
p->locations.frag.lights[i].enabled =                                         \
    program_get_uniform_location(p->program.gl_id, "lights["#i"].enabled");

    POINT_LIGHT(0);
    POINT_LIGHT(1);
    POINT_LIGHT(2);
    POINT_LIGHT(3);
    RICO_ASSERT(4 == NUM_LIGHTS);
#undef POINT_LIGHT

    RICO_ASSERT(p->locations.frag.camera.pos >= 0);
    RICO_ASSERT(p->locations.frag.material.tex0 >= 0);
    RICO_ASSERT(p->locations.frag.material.tex1 >= 0);
    RICO_ASSERT(p->locations.frag.material.tex2 >= 0);
    RICO_ASSERT(p->locations.frag.lights[0].pos >= 0);
    RICO_ASSERT(p->locations.frag.lights[0].color >= 0);
    RICO_ASSERT(p->locations.frag.lights[0].intensity >= 0);
    RICO_ASSERT(p->locations.frag.lights[0].enabled >= 0);
    // TODO: Turn these back on when they're being used
    //RICO_ASSERT(p->locations.frag.light.enabled >= 0);
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
    err = make_program(vshader, 0, fshader, &program);
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
//| Shadow program
///=============================================================================
static void program_shadow_get_locations(struct shadow_program *p)
{
    // Vertex shader
    p->locations.vert.model =
        program_get_uniform_location(p->program.gl_id, "model");
    p->locations.vert.attrs.position =
        program_get_attrib_location(p->program.gl_id,"attr_position");

    RICO_ASSERT(p->locations.vert.model >= 0);
    RICO_ASSERT(p->locations.vert.attrs.position == LOCATION_PBR_POSITION);

    // Geometry shader
#define CUBEMAP_XFORM(i)                                                    \
p->locations.geom.cubemap_xforms[i] =                                       \
    program_get_uniform_location(p->program.gl_id, "cubemap_xforms["#i"]");

    CUBEMAP_XFORM(0);
    CUBEMAP_XFORM(1);
    CUBEMAP_XFORM(2);
    CUBEMAP_XFORM(3);
    CUBEMAP_XFORM(4);
    CUBEMAP_XFORM(5);
#undef CUBEMAP_XFORM

    RICO_ASSERT(p->locations.geom.cubemap_xforms[0] >= 0);
    RICO_ASSERT(p->locations.geom.cubemap_xforms[1] >= 0);
    RICO_ASSERT(p->locations.geom.cubemap_xforms[2] >= 0);
    RICO_ASSERT(p->locations.geom.cubemap_xforms[3] >= 0);
    RICO_ASSERT(p->locations.geom.cubemap_xforms[4] >= 0);
    RICO_ASSERT(p->locations.geom.cubemap_xforms[5] >= 0);

    // Fragment shader
    p->locations.frag.far_plane =
        program_get_uniform_location(p->program.gl_id, "far_plane");
    p->locations.frag.light_pos =
        program_get_uniform_location(p->program.gl_id, "light_pos");

    RICO_ASSERT(p->locations.frag.far_plane >= 0);
    RICO_ASSERT(p->locations.frag.light_pos >= 0);
}
static void program_shadow_attribs()
{
    glVertexAttribPointer(LOCATION_SHADOW_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, pos));
    glEnableVertexAttribArray(LOCATION_SHADOW_POSITION);
}
static int make_program_shadow(struct shadow_program **_program)
{
    static struct shadow_program *prog_shadow = NULL;
    enum RICO_error err;

    if (prog_shadow != NULL) {
        *_program = prog_shadow;
        return SUCCESS;
    }

    GLuint vshader = 0;
    GLuint gshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    // Compile shaders
    err = make_shader(GL_VERTEX_SHADER, "shader/shadow_v.glsl", &vshader);
    if (err) goto cleanup;

    err = make_shader(GL_GEOMETRY_SHADER, "shader/shadow_g.glsl", &gshader);
    if (err) goto cleanup;

    err = make_shader(GL_FRAGMENT_SHADER, "shader/shadow_f.glsl", &fshader);
    if (err) goto cleanup;

    // Link shader program
    err = make_program(vshader, gshader, fshader, &program);
    if (err) goto cleanup;

    // Create program object
    prog_shadow = calloc(1, sizeof(*prog_shadow));
    prog_shadow->program.type = PROG_SHADOW;
    prog_shadow->program.gl_id = program;

    // Query shader locations
    program_shadow_get_locations(prog_shadow);

cleanup:
    free_shader(fshader);
    free_shader(gshader);
    free_shader(vshader);
    *_program = prog_shadow;
    return err;
}
static void free_program_shadow(struct shadow_program **program)
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
    p->locations.vert.proj =
        program_get_uniform_location(p->program.gl_id, "proj");
    p->locations.vert.view =
        program_get_uniform_location(p->program.gl_id, "view");
    p->locations.vert.model =
        program_get_uniform_location(p->program.gl_id, "model");
    RICO_ASSERT(p->locations.vert.proj >= 0);
    RICO_ASSERT(p->locations.vert.view >= 0);
    RICO_ASSERT(p->locations.vert.model >= 0);

    p->locations.vert.attrs.position =
        program_get_attrib_location(p->program.gl_id, "attr_position");
    p->locations.vert.attrs.uv =
        program_get_attrib_location(p->program.gl_id, "attr_uv");
    p->locations.vert.attrs.color =
        program_get_attrib_location(p->program.gl_id, "attr_color");
    RICO_ASSERT(p->locations.vert.attrs.position == LOCATION_PRIM_POSITION);
    // TODO: Turn these back on when they're being used
    //RICO_ASSERT(p->locations.vert.attrs.uv == LOCATION_PRIM_UV);
    //RICO_ASSERT(p->locations.vert.attrs.color == LOCATION_PRIM_COLOR);

    // Fragment shader
    p->locations.frag.color =
        program_get_uniform_location(p->program.gl_id, "color");
    p->locations.frag.tex0 =
        program_get_uniform_location(p->program.gl_id, "tex");
    // TODO: Turn these back on when they're being used
    //RICO_ASSERT(p->locations.frag.color >= 0);
    //RICO_ASSERT(p->locations.frag.tex >= 0);
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
    err = make_program(vshader, 0, fshader, &program);
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
    p->locations.vert.model =
        program_get_uniform_location(p->program.gl_id, "model");
    p->locations.vert.view =
        program_get_uniform_location(p->program.gl_id, "view");
    p->locations.vert.proj =
        program_get_uniform_location(p->program.gl_id, "proj");
    RICO_ASSERT(p->locations.vert.model >= 0);
    RICO_ASSERT(p->locations.vert.view >= 0);
    RICO_ASSERT(p->locations.vert.proj >= 0);

    // Fragment shader
    p->locations.frag.color =
        program_get_uniform_location(p->program.gl_id, "u_color");
    p->locations.frag.grayscale =
        program_get_uniform_location(p->program.gl_id, "u_grayscale");
    p->locations.frag.tex0 =
        program_get_uniform_location(p->program.gl_id, "u_tex");
    //RICO_ASSERT(p->locations.frag.color >= 0);
    //RICO_ASSERT(p->locations.frag.grayscale >= 0);
    //RICO_ASSERT(p->locations.frag.tex >= 0);
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
    err = make_program(vshader, 0, fshader, &program);
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
