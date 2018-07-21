#define LIGHT_NEAR 0.01f
#define LIGHT_FAR 25.0f

static GLuint shadow_textures[NUM_LIGHT_DIR] = { 0 };
static GLuint shadow_cubemaps[NUM_LIGHT_POINT] = { 0 };
static struct mat4 debug_sun_xform = { 0 };
static struct mat4 shadow_lightspace[NUM_LIGHT_DIR] = { 0 };
static struct mat4 shadow_ortho = { 0 };
static struct mat4 shadow_proj = { 0 };

static void create_shadow_texture(GLuint *tex_id, u32 size)
{
    glGenTextures(1, tex_id);
    glBindTexture(GL_TEXTURE_2D, *tex_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size, size, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

static void create_shadow_cubemap(GLuint *tex_id, u32 size)
{
    glGenTextures(1, tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *tex_id);

    // Generate the 6 face textures for the cube map
    for (GLint i = 0; i < 6; ++i)
    {
        //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
        //             GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT,
        //             GL_UNSIGNED_INT, NULL);

        // If need more precision (slides):
        //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
        //             GL_DEPTH_COMPONENT32F, w, h, 0, GL_DEPTH_COMPONENT,
        //             GL_FLOAT, NULL);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                     GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT,
                     GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Cleanup: Check if these have any effect after shadows are fixed
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

static void create_framebuffer(GLuint *fbo_id, GLuint tex_id)
{
    glGenFramebuffers(1, fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo_id);

    // Attach shadow map to framebuffer depth
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_id, 0);

    // We don't care about color buffers for shadow map
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_shadow_cubemap(r64 alpha, struct RICO_light *lights)
{
    const u32 tex_size = 1024;

    // TODO: Refactor this debauchery
    static struct mat4 rot_matrices[6] = { 0 };
    static GLuint shadow_fbo[NUM_LIGHT_DIR + NUM_LIGHT_POINT] = { 0 };
    struct mat4 light_views[6] = { 0 };

    if (!shadow_textures[0])
    {
        for (int i = 0; i < NUM_LIGHT_DIR; ++i)
        {
            create_shadow_texture(&shadow_textures[i], tex_size);
            create_framebuffer(&shadow_fbo[i], shadow_textures[i]);
        }
        for (int i = 0; i < NUM_LIGHT_POINT; ++i)
        {
            create_shadow_cubemap(&shadow_cubemaps[i], tex_size);
            create_framebuffer(&shadow_fbo[NUM_LIGHT_DIR + i],
                               shadow_cubemaps[i]);
        }

        // Calculate rotation matrices
        //rot_matrices[0] = mat4_init_rotx( 90.0f); // pos_x
        //rot_matrices[1] = mat4_init_rotx(-90.0f); // neg_x
        //rot_matrices[2] = mat4_init_roty( 90.0f); // pos_y
        //rot_matrices[3] = mat4_init_roty(-90.0f); // neg_y
        //rot_matrices[4] = mat4_init_rotz( 90.0f); // pos_z
        //rot_matrices[5] = mat4_init_rotz(-90.0f); // neg_z
        rot_matrices[0] = mat4_init(
            0.0f,  0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[1] = mat4_init(
            0.0f,  0.0f,  1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[2] = mat4_init(
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[3] = mat4_init(
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f,  0.0f, -1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[4] = mat4_init(
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
            0.0f,  0.0f, -1.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );
        rot_matrices[5] = mat4_init(
           -1.0f,  0.0f,  0.0f,  0.0f,
            0.0f, -1.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  1.0f,  0.0f,
            0.0f,  0.0f,  0.0f,  1.0f
        );

        // Cleanup: Print rotation matrices to log file
        //for (int i = 0; i < 6; i++)
        //{
        //    printf("rot_matrix[%d]\n", i);
        //    mat4_print(&rot_matrices[i]);
        //    fflush(stdout);
        //}

        float ortho_width = 50.0f;
        float ortho_far = 250.0f;
        shadow_ortho = mat4_init_ortho(-ortho_width, ortho_width, ortho_width,
                                       -ortho_width, ortho_far, -ortho_far);

        // Calculate projection matrix
        //new THREE.PerspectiveCamera(90, 1, 0.01, 1000).projectionMatrix
        shadow_proj = mat4_init_perspective(1.0f, LIGHT_NEAR, 0.0f, 90.0f);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Render shadows
    glViewport(0, 0, tex_size, tex_size);

    // Directional light shadows
    struct shadow_texture_program *prog_texture = prog_shadow_texture;
    glUseProgram(prog_texture->program.gl_id);

    for (int light = 0; light < NUM_LIGHT_DIR; ++light)
    {
        if (!lights[light].on)
            continue;

        // projection * view
        struct mat4 proj = shadow_ortho;

        struct vec3 light_view = lights[light].directional.dir;
        struct mat4 view = mat4_init_lookat(&light_view, &VEC3_ZERO, &VEC3_UP);
        mat4_mul(&proj, &view);
        shadow_lightspace[light] = proj;
        glUniformMatrix4fv(prog_texture->locations.vert.light_space, 1, GL_TRUE,
                           shadow_lightspace[light].a);

        // TODO: Cleanup
        debug_sun_xform = shadow_lightspace[light];

        // Clear depth buffer and bind shadow map
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo[light]);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Render scene to this light's shadow map
        for (u32 pack = 1; pack < ARRAY_COUNT(packs); ++pack)
        {
            if (!packs[pack])
                continue;

            object_render(packs[pack], prog_texture->locations.vert.model, 0,
                          true);
        }
    }

    // Point light shadows
    struct shadow_cubemap_program *prob_cube = prog_shadow_cubemap;
    glUseProgram(prob_cube->program.gl_id);
    glUniform2f(prob_cube->locations.frag.near_far, LIGHT_NEAR, LIGHT_FAR);

    for (int light = NUM_LIGHT_DIR; light < NUM_LIGHT_DIR + NUM_LIGHT_POINT;
         ++light)
    {
        if (!lights[light].on)
            continue;

        glUniform3fv(prob_cube->locations.frag.light_pos, 1,
                     &lights[light].pos.x);
        struct vec3 light_pos_neg = lights[light].pos;
        v3_negate(&light_pos_neg);

        // projection * rotate * translate
        for (int view = 0; view < 6; ++view)
        {
            light_views[view] = MAT4_IDENT;
            mat4_mul(&light_views[view], &shadow_proj);
            mat4_mul(&light_views[view], &rot_matrices[view]);
            mat4_translate(&light_views[view], &light_pos_neg);

            glUniformMatrix4fv(prob_cube->locations.geom.cubemap_xforms[view],
                               1, GL_TRUE, light_views[view].a);
        }

        // Clear depth buffer and bind shadow map
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo[light]);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Render scene to this light's shadow map
        for (u32 pack = 1; pack < ARRAY_COUNT(packs); ++pack)
        {
            if (!packs[pack])
                continue;

            object_render(packs[pack], prob_cube->locations.vert.model, 0,
                          true);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}