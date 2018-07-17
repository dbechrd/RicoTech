static program_attribs_helper program_attribs[PROG_COUNT] =
{
    0,
    program_pbr_attribs,
    program_shadow_texture_attribs,
    program_shadow_cubemap_attribs,
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

#define LIGHT(i)                                                              \
p->locations.frag.lights[i].pos =                                             \
    program_get_uniform_location(p->program.gl_id, "lights["#i"].pos");       \
p->locations.frag.lights[i].dir =                                             \
    program_get_uniform_location(p->program.gl_id, "lights["#i"].dir");       \
p->locations.frag.lights[i].color =                                           \
    program_get_uniform_location(p->program.gl_id, "lights["#i"].color");     \
p->locations.frag.lights[i].intensity =                                       \
    program_get_uniform_location(p->program.gl_id, "lights["#i"].intensity"); \
p->locations.frag.lights[i].enabled =                                         \
    program_get_uniform_location(p->program.gl_id, "lights["#i"].enabled");

    LIGHT(0);
    LIGHT(1);
    LIGHT(2);
    LIGHT(3);
    LIGHT(4);
    RICO_ASSERT(5 == NUM_LIGHT_DIR + NUM_LIGHT_POINT);
#undef POINT_LIGHT

    RICO_ASSERT(p->locations.frag.camera.pos >= 0);
    RICO_ASSERT(p->locations.frag.material.tex0 >= 0);
    RICO_ASSERT(p->locations.frag.material.tex1 >= 0);
    RICO_ASSERT(p->locations.frag.material.tex2 >= 0);
    //RICO_ASSERT(p->locations.frag.lights[0].pos >= 0);
    //RICO_ASSERT(p->locations.frag.lights[0].dir >= 0);
    RICO_ASSERT(p->locations.frag.lights[0].color >= 0);
    RICO_ASSERT(p->locations.frag.lights[0].intensity >= 0);
    RICO_ASSERT(p->locations.frag.lights[0].enabled >= 0);

    p->locations.frag.near_far =
        program_get_uniform_location(p->program.gl_id, "near_far");

#define SHADOW_TEXTURE(i)                                                    \
p->locations.frag.shadow_textures[i] =                                       \
    program_get_uniform_location(p->program.gl_id, "shadow_textures["#i"]");

    SHADOW_TEXTURE(0);
    RICO_ASSERT(1 == NUM_LIGHT_DIR);
    //RICO_ASSERT(p->locations.frag.shadow_textures[0] >= 0);
#undef SHADOW_TEXTURE

#define SHADOW_LIGHTSPACE(i)                                                 \
p->locations.frag.shadow_lightspace[i] =                                     \
    program_get_uniform_location(p->program.gl_id, "shadow_lightspace["#i"]");

    SHADOW_LIGHTSPACE(0);
    RICO_ASSERT(1 == NUM_LIGHT_DIR);
    //RICO_ASSERT(p->locations.frag.shadow_lightspace[0] >= 0);
#undef SHADOW_LIGHTSPACE

#define SHADOW_CUBEMAP(i)                                                    \
p->locations.frag.shadow_cubemaps[i] =                                       \
    program_get_uniform_location(p->program.gl_id, "shadow_cubemaps["#i"]");

    SHADOW_CUBEMAP(0);
    SHADOW_CUBEMAP(1);
    SHADOW_CUBEMAP(2);
    SHADOW_CUBEMAP(3);
    RICO_ASSERT(4 == NUM_LIGHT_POINT);
    //RICO_ASSERT(p->locations.frag.shadow_cubemaps[0] >= 0);
#undef SHADOW_CUBEMAP

    p->locations.frag.light_proj =
        program_get_uniform_location(p->program.gl_id, "light_proj");
    //RICO_ASSERT(p->locations.frag.light_proj >= 0);
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
    static struct pbr_program *prog = NULL;
    enum RICO_error err;

    if (prog != NULL) {
        *_program = prog;
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
    prog = calloc(1, sizeof(*prog));
    prog->program.type = PROG_PBR;
    prog->program.gl_id = program;

    // Query shader locations
    program_pbr_get_locations(prog);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog;
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
//| Shadow texture program
///=============================================================================
static void program_shadow_texture_get_locations(
    struct shadow_texture_program *p)
{
    // Vertex shader
    p->locations.vert.light_space =
        program_get_uniform_location(p->program.gl_id, "light_space");
    p->locations.vert.model =
        program_get_uniform_location(p->program.gl_id, "model");
    p->locations.vert.attrs.position =
        program_get_attrib_location(p->program.gl_id,"attr_position");

    RICO_ASSERT(p->locations.vert.model >= 0);
    RICO_ASSERT(p->locations.vert.attrs.position == LOCATION_PBR_POSITION);

    // Fragment shader
    //Empty
}
static void program_shadow_texture_attribs()
{
    glVertexAttribPointer(LOCATION_SHADOW_TEXTURE_POSITION, 3, GL_FLOAT,
                          GL_FALSE, sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, pos));
    glEnableVertexAttribArray(LOCATION_SHADOW_TEXTURE_POSITION);
}
static int make_program_shadow_texture(struct shadow_texture_program **_program)
{
    static struct shadow_texture_program *prog = NULL;
    enum RICO_error err;

    if (prog != NULL) {
        *_program = prog;
        return SUCCESS;
    }

    GLuint vshader = 0;
    GLuint gshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    // Compile shaders
    err = make_shader(GL_VERTEX_SHADER, "shader/shadow_texture_v.glsl",
                      &vshader);
    if (err) goto cleanup;

    err = make_shader(GL_FRAGMENT_SHADER, "shader/shadow_texture_f.glsl",
                      &fshader);
    if (err) goto cleanup;

    // Link shader program
    err = make_program(vshader, gshader, fshader, &program);
    if (err) goto cleanup;

    // Create program object
    prog = calloc(1, sizeof(*prog));
    prog->program.type = PROG_SHADOW_CUBEMAP;
    prog->program.gl_id = program;

    // Query shader locations
    program_shadow_texture_get_locations(prog);

cleanup:
    free_shader(fshader);
    free_shader(gshader);
    free_shader(vshader);
    *_program = prog;
    return err;
}
static void free_program_shadow_texture(struct shadow_texture_program **program)
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
//| Shadow cubemap program
///=============================================================================
static void program_shadow_cubemap_get_locations(
    struct shadow_cubemap_program *p)
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
    p->locations.frag.near_far =
        program_get_uniform_location(p->program.gl_id, "near_far");
    p->locations.frag.light_pos =
        program_get_uniform_location(p->program.gl_id, "light_pos");
    p->locations.frag.light_dir =
        program_get_uniform_location(p->program.gl_id, "light_dir");

    RICO_ASSERT(p->locations.frag.near_far >= 0);
    RICO_ASSERT(p->locations.frag.light_pos >= 0);
    RICO_ASSERT(p->locations.frag.light_dir >= 0);
}
static void program_shadow_cubemap_attribs()
{
    glVertexAttribPointer(LOCATION_SHADOW_CUBEMAP_POSITION, 3, GL_FLOAT,
                          GL_FALSE, sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, pos));
    glEnableVertexAttribArray(LOCATION_SHADOW_CUBEMAP_POSITION);
}
static int make_program_shadow_cubemap(struct shadow_cubemap_program **_program)
{
    static struct shadow_cubemap_program *prog = NULL;
    enum RICO_error err;

    if (prog != NULL) {
        *_program = prog;
        return SUCCESS;
    }

    GLuint vshader = 0;
    GLuint gshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    // Compile shaders
    err = make_shader(GL_VERTEX_SHADER, "shader/shadow_cubemap_v.glsl",
                      &vshader);
    if (err) goto cleanup;

    err = make_shader(GL_GEOMETRY_SHADER, "shader/shadow_cubemap_g.glsl",
                      &gshader);
    if (err) goto cleanup;

    err = make_shader(GL_FRAGMENT_SHADER, "shader/shadow_cubemap_f.glsl",
                      &fshader);
    if (err) goto cleanup;

    // Link shader program
    err = make_program(vshader, gshader, fshader, &program);
    if (err) goto cleanup;

    // Create program object
    prog = calloc(1, sizeof(*prog));
    prog->program.type = PROG_SHADOW_CUBEMAP;
    prog->program.gl_id = program;

    // Query shader locations
    program_shadow_cubemap_get_locations(prog);

cleanup:
    free_shader(fshader);
    free_shader(gshader);
    free_shader(vshader);
    *_program = prog;
    return err;
}
static void free_program_shadow_cubemap(struct shadow_cubemap_program **program)
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
static void program_primitive_get_locations(struct primitive_program *p)
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
static int make_program_primitive(struct primitive_program **_program)
{
    static struct primitive_program *prog = NULL;
    enum RICO_error err;

    if (prog != NULL) {
        *_program = prog;
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
    prog = calloc(1, sizeof(*prog));
    prog->program.type = PROG_PRIMITIVE;
    prog->program.gl_id = program;

    // Query shader locations
    program_primitive_get_locations(prog);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog;
    return err;
}
static void free_program_primitive(struct primitive_program **program)
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
    static struct text_program *prog = NULL;
    enum RICO_error err;

    if (prog != NULL) {
        *_program = prog;
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
    prog = calloc(1, sizeof(*prog));
    prog->program.type = PROG_TEXT;
    prog->program.gl_id = program;

    // Query shader locations
    program_text_get_locations(prog);

cleanup:
    free_shader(fshader);
    free_shader(vshader);
    *_program = prog;
    return err;
}
static void free_program_text(struct text_program **program)
{
    glDeleteProgram((*program)->program.gl_id);
    free(*program);
    *program = NULL;
}
