#define HEIRO_HEIGHT 16

static GLuint glyph_vao;
static GLuint glyph_vbo;

#define HEIRO_CODEPOINT_FIRST (u32)' '
#define HEIRO_CODEPOINT_LAST (u32)'~'

// TODO: Use hash table instead
static struct RICO_heiro_glyph glyphs[(HEIRO_CODEPOINT_LAST + 1) -
                                      HEIRO_CODEPOINT_FIRST];
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
    FT_TRY(FT_Set_Pixel_Sizes(ft_face, 0, HEIRO_HEIGHT), ERR_FREETYPE_SIZE);

    // Set alignment to 1 byte for 8-bit grayscale glyphs
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct text_vertex[6]), 0,
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
#define ATLAS_WIDTH 256
#define ATLAS_HEIGHT 256
#define ATLAS_PAD 1

    enum rico_error err = SUCCESS;

    u32 atlas_x = 0;
    u32 atlas_y = 0;
    u32 row_max_y = 0;
    u8 *atlas = calloc(1, sizeof(u8) * ATLAS_WIDTH * ATLAS_HEIGHT);

    FT_ULong c;
    for (c = 0; c < ARRAY_COUNT(glyphs); ++c)
    {
        u8 *buffer;
        err = heiro_load_glyph(&buffer, &glyphs[c], ft_face,
                               HEIRO_CODEPOINT_FIRST + c);
        // TODO: Use placeholder character when glyph is missing and keep going
        if (err) break;

        if (!buffer) continue;

        u32 w = glyphs[c].width + ATLAS_PAD;
        u32 h = glyphs[c].height + ATLAS_PAD;

        // Store max glyph height so we know how much to advance y at end of row
        if (glyphs[c].height > row_max_y)
        {
            row_max_y = h;
        }

        // If texture doesn't fit, start a new row
        if (atlas_x + w > ATLAS_WIDTH)
        {
            atlas_x = 0;
            atlas_y += row_max_y;
            row_max_y = 0;
        }

        // TODO: Resize texture or create a second one
        // If texture still doesn't fit, uh-oh
        if (atlas_x + w > ATLAS_WIDTH || atlas_y + h > ATLAS_HEIGHT)
        {
            RICO_ASSERT(0);  // Texture not big enough to hold font glyphs
        }

        glyphs[c].uvs[0].u = ((float)atlas_x + ATLAS_PAD) / ATLAS_WIDTH;
        glyphs[c].uvs[0].v = ((float)atlas_y + ATLAS_PAD) / ATLAS_HEIGHT;
        glyphs[c].uvs[1].u = (float)(atlas_x + w) / ATLAS_WIDTH;
        glyphs[c].uvs[1].v = (float)(atlas_y + h) / ATLAS_HEIGHT;

        // Copy glyph into texture buffer
        for (u32 y = 0; y < h; ++y)
        {
            for (u32 x = 0; x < w; ++x)
            {
                u32 dst_x = atlas_x + x;
                u32 dst_y = atlas_y + y;
                u8 *dst = atlas + (dst_y * ATLAS_WIDTH) + dst_x;

                // Pad trailing edges with black
                if (x < ATLAS_PAD || y < ATLAS_PAD)
                {
                    *dst = 0;
                    continue;
                }

                u32 src_x = x - ATLAS_PAD;
                u32 src_y = y - ATLAS_PAD;
                u8 *src = buffer + (src_y * glyphs[c].width) + src_x;

                *dst = *src;
            }
        }

        atlas_x += w;
    }

    GLuint gl_id = heiro_upload_texture(ATLAS_WIDTH, ATLAS_HEIGHT, atlas);
    for (FT_ULong c = 0; c < ARRAY_COUNT(glyphs); ++c)
    {
        glyphs[c].gl_id = gl_id;
    }

    free(atlas);
    return err;

#undef TEX_WIDTH
#undef TEX_HEIGHT
#undef ATLAS_PAD
}

int heiro_load_glyph(u8 **buffer, struct RICO_heiro_glyph *glyph,
                     FT_Face ft_face, FT_ULong char_code)
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
    glyph->advance_x    = ft_face->glyph->advance.x;
    glyph->advance_y    = ft_face->glyph->advance.y;

    *buffer = ft_face->glyph->bitmap.buffer;

cleanup:
    return err;
}

// TODO: Refactor into upload_texture w/ options for wrap, filter, pixel size
//void heiro_upload_glyph(struct RICO_heiro_glyph *glyph, const void *pixels)
GLuint heiro_upload_texture(s32 width, s32 height, const void *pixels)
{
    GLuint gl_id;
    //glCreateTextures(GL_TEXTURE_2D, 1, &glyph->gl_id);
    glGenTextures(1, &gl_id);
    glBindTexture(GL_TEXTURE_2D, gl_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED,
                 GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return gl_id;
}

extern void RICO_heiro_render(s32 sx, s32 sy, const u8 *str, u32 len)
{
    struct text_program *prog = prog_text;
    RICO_ASSERT(prog->program.gl_id);
    glUseProgram(prog->program.gl_id);

    glUniformMatrix4fv(prog->vert.proj, 1, GL_TRUE, cam_player.ortho_matrix.a);
    glUniformMatrix4fv(prog->vert.view, 1, GL_TRUE, MAT4_IDENT.a);
    glUniformMatrix4fv(prog->vert.model, 1, GL_TRUE, MAT4_IDENT.a);
    
    glUniform4fv(prog->frag.color, 1, &COLOR_WHITE.r);
    glUniform1i(prog->frag.grayscale, true);
    glUniform1i(prog->frag.tex, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(glyph_vao);

    float x = (float)sx;
    float y = (float)sy;
    float scale = 1.0f;

    struct vec4 color = COLOR_BLUE;
    struct text_vertex vertices[4];

    u32 prev_tex = 0;
    for (u32 i = 0; i < len; ++i)
    {
        if (str[i] == '\n')
        {
            x = (float)sx;
            y += HEIRO_HEIGHT;
            continue;
        }

        if (str[i] < HEIRO_CODEPOINT_FIRST || str[i] > HEIRO_CODEPOINT_LAST)
        {
            continue;
        }
        //RICO_ASSERT(str[i] >= HEIRO_CODEPOINT_FIRST);
        struct RICO_heiro_glyph *glyph = &glyphs[str[i] - HEIRO_CODEPOINT_FIRST];

        float xpos = x + glyph->bearing_left;
        float ypos = (y - glyph->height) +
            ((float)glyph->height - glyph->bearing_top);

        float w = glyph->width * scale;
        float h = glyph->height * scale;

        float x0 = X_TO_NDC(xpos);
        float y0 = Y_TO_NDC(ypos);
        float x1 = SCREEN_X(xpos + w);
        float y1 = SCREEN_Y(ypos + h);
        float z = -1.0f;

        float u0 = glyph->uvs[0].u;
        float v0 = glyph->uvs[0].v;
        float u1 = glyph->uvs[1].u;
        float v1 = glyph->uvs[1].v;

        vertices[0] = (struct text_vertex) {
            VEC3(x0, y0, z), VEC2F(u0, v0), color
        };
        vertices[1] = (struct text_vertex) {
            VEC3(x0, y1, z), VEC2F(u0, v1), color
        };
        vertices[2] = (struct text_vertex) {
            VEC3(x1, y0, z), VEC2F(u1, v0), color
        };
        vertices[3] = (struct text_vertex) {
            VEC3(x1, y1, z), VEC2F(u1, v1), color
        };

        if (glyph->gl_id != prev_tex)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, glyph->gl_id);
            prev_tex = glyph->gl_id;
        }
        glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        u32 adv = (glyph->advance_x >> 6);
        x += adv * scale;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
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