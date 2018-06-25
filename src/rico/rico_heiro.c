static GLuint glyph_vao;
static GLuint glyph_vbo;

// TODO: Use hash table instead
static struct RICO_heiro_glyph glyphs[128];
//static struct dlb_hash glyphs;

#define FT_TRY(expr, code) \
    ft_err = (expr); \
    if (ft_err) { \
        err = RICO_ERROR(code, "%d %s", ft_err, getErrorMessage(ft_err)); \
        goto cleanup; \
    }

const char* getErrorMessage(FT_Error err);

int rico_heiro_init()
{
    enum rico_error err;
    FT_Error ft_err;

    FT_Library ft_lib = 0;
    FT_Face ft_face = 0;

    FT_TRY(FT_Init_FreeType(&ft_lib), ERR_FREETYPE_INIT);
    FT_TRY(FT_New_Face(ft_lib, "font/Cousine-Regular.ttf", 0, &ft_face),
           ERR_FREETYPE_FACE);
    // Note: Use this if the font data is already loaded into a memory buffer
    //FREETYPE_TRY(FT_New_Memory_Face(ft_lib, buffer, buffer_size, 0, &ft_face),
    //             ERR_FREETYPE_FACE;
    FT_TRY(FT_Set_Pixel_Sizes(ft_face, 0, 14), ERR_FREETYPE_SIZE);

    // TODO: Do I need to set cmap or encoding?
    //FT_Set_Charmap(ft_face, charmap);
    //FT_Select_Charmap(ft_face, encoding);

    err = heiro_load_glyphs(ft_face);
    if (err) goto cleanup;

    // Generate glyph vao/vbo
    glGenVertexArrays(1, &glyph_vao);
    glGenBuffers(1, &glyph_vbo);

    // Bind vao/vbo
    glBindVertexArray(glyph_vao);
    glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);

    // Reserve enough buffer space for a single glyph quad's vertices
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct prim_vertex[6]), 0,
                 GL_DYNAMIC_DRAW);
    program_attribs[PROG_PRIM]();
    // Cleanup: Placeholder
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    // Unbind vbo/vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

cleanup:
    if (ft_face) FT_Done_Face(ft_face);
    if (ft_lib) FT_Done_FreeType(ft_lib);
    return err;
}

int heiro_load_glyphs(FT_Face ft_face)
{
    enum rico_error err = SUCCESS;
    
    // Set alignment to 1 byte for 8-bit grayscale glyphs
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (FT_ULong c = 0; c < ARRAY_COUNT(glyphs); ++c)
    {
        err = heiro_load_glyph(&glyphs[c], ft_face, c);
        
        // TODO: Use placeholder character when glyph is missing and keep going
        if (err) break;
    }

    // Reset default alignment
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    return err;
}

int heiro_load_glyph(struct RICO_heiro_glyph *glyph, FT_Face ft_face,
                     FT_ULong char_code)
{
    enum rico_error err = SUCCESS;
    FT_Error ft_err;

    // Note: Looks up glyph based on on current charmap
    FT_TRY(FT_Load_Char(ft_face, char_code, FT_LOAD_RENDER), ERR_FREETYPE_CHAR);
    //FT_TRY(FT_Load_Glyph(ft_face, glyph_code, FT_LOAD_RENDER);

    glyph->width        = ft_face->glyph->bitmap.width;
    glyph->height       = ft_face->glyph->bitmap.rows;
    glyph->bearing_left = ft_face->glyph->bitmap_left;
    glyph->bearing_top  = ft_face->glyph->bitmap_top;
    glyph->advance      = ft_face->glyph->advance.x;

    heiro_upload_glyph(glyph, ft_face->glyph->bitmap.buffer);

cleanup:
    return err;
}

void heiro_upload_glyph(struct RICO_heiro_glyph *glyph, const void *pixels)
{
    //glCreateTextures(GL_TEXTURE_2D, 1, &glyph->gl_id);
    glGenTextures(1, &glyph->gl_id);
    glBindTexture(GL_TEXTURE_2D, glyph->gl_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glyph->width, glyph->height, 0,
                 GL_RED, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

extern void RICO_heiro_render_string(s32 sx, s32 sy, const u8 *str, u32 len)
{
    struct prim_program *prog = prog_prim;
    RICO_ASSERT(prog->program.gl_id);
    glUseProgram(prog->program.gl_id);

    glUniformMatrix4fv(prog->vert.proj, 1, GL_TRUE, cam_player.ortho_matrix.a);
    glUniformMatrix4fv(prog->vert.model, 1, GL_TRUE, MAT4_IDENT.a);
    
    glUniform1i(prog->frag.tex, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(glyph_vao);

    float x = (float)sx;
    float y = (float)sy;
    float scale = 1.0f;

    struct vec4 color = COLOR_TRANSPARENT;
    struct prim_vertex vertices[6];

    u32 prev_tex = 0;
    for (u32 i = 0; i < len; ++i)
    {
        struct RICO_heiro_glyph *glyph = &glyphs[str[i]];

        float xpos = x + glyph->bearing_left;
        float ypos = (y - glyph->height) + (glyph->height - glyph->bearing_top);

        float w = glyph->width * scale;
        float h = glyph->height * scale;

        float x0 = X_TO_NDC(xpos);
        float y0 = Y_TO_NDC(ypos);
        float x1 = SCREEN_X(xpos + w);
        float y1 = SCREEN_Y(ypos + h);
        float z = -1.0f;

        float u0 = 0.0f;
        float u1 = 1.0f;
        float v0 = 1.0f;
        float v1 = 0.0f;

        vertices[0] = (struct prim_vertex) {
            VEC3(x0, y1, z), VEC2F(u0, v0), color
        };
        vertices[1] = (struct prim_vertex) {
            VEC3(x1, y0, z), VEC2F(u1, v1), color
        };
        vertices[2] = (struct prim_vertex) {
            VEC3(x0, y0, z), VEC2F(u0, v1), color
        };
        vertices[3] = (struct prim_vertex) {
            VEC3(x0, y1, z), VEC2F(u0, v0), color
        };
        vertices[4] = (struct prim_vertex) {
            VEC3(x1, y1, z), VEC2F(u1, v0), color
        };
        vertices[5] = (struct prim_vertex) {
            VEC3(x1, y0, z), VEC2F(u1, v1), color
        };

        if (glyph->gl_id != prev_tex)
        {
            glBindTexture(GL_TEXTURE_2D, glyph->gl_id);
            prev_tex = glyph->gl_id;
        }
        glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        u32 adv = (glyph->advance >> 6);
        x += adv * scale;
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void heiro_delete_glyphs()
{
    for (u32 i = 0; i < ARRAY_COUNT(glyphs); ++i)
    {
        if (glyphs[i].gl_id)
        {
            heiro_delete_glyph(&glyphs[i]);
        }
    }
    // TODO: Assume glyphs is only ever freed when program exits, so we don't
    //       need to zero the pool.
    //memset(glyphs, 0, sizeof(glyphs));
}

void heiro_delete_glyph(struct RICO_heiro_glyph *glyph)
{
    glDeleteTextures(1, &glyph->gl_id);
}

void heiro_free()
{
    heiro_delete_glyphs();
}

const char* getErrorMessage(FT_Error err)
{
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
    return "Unknown error code";
}