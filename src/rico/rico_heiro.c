#define HEIRO_MAX_LEN 1024
#define HEIRO_GLYPH_VERTS 6

static GLuint glyph_vao;
static GLuint glyph_vbo;

#define HEIRO_CODEPOINT_FIRST (u32)' '
#define HEIRO_CODEPOINT_LAST (u32)'~'

static struct rect glyph_max_extents;

// TODO: Use hash table instead
static struct ric_heiro_glyph glyphs[(HEIRO_CODEPOINT_LAST + 1) -
                                      HEIRO_CODEPOINT_FIRST];
//static dlb_hash glyphs;

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

    FT_TRY(FT_Init_FreeType(&ft_lib), RIC_ERR_FREETYPE_INIT);
    FT_TRY(FT_New_Face(ft_lib, "font/Cousine-Regular.ttf", 0, &ft_face),
           RIC_ERR_FREETYPE_FACE);
    // Note: Use this if the font data is already loaded into a memory buffer
    //FREETYPE_TRY(FT_New_Memory_Face(ft_lib, buffer, buffer_size, 0, &ft_face),
    //             RIC_ERR_FREETYPE_FACE;
    FT_TRY(FT_Set_Pixel_Sizes(ft_face, 0, HEIRO_GLYPH_HEIGHT),
           RIC_ERR_FREETYPE_SIZE);

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
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(struct text_vertex[HEIRO_MAX_LEN * HEIRO_GLYPH_VERTS ]),
                 0, GL_DYNAMIC_DRAW);
    program_attribs[PROG_PRIMITIVE]();

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
#define ATLAS_WIDTH 128
#define ATLAS_HEIGHT 128
#define ATLAS_PAD 1

    enum rico_error err = RIC_SUCCESS;

    u32 atlas_x = 0;
    u32 atlas_y = 0;
    u32 row_max_y = 0;
    u8 *atlas = calloc(1, sizeof(u8) * ATLAS_WIDTH * ATLAS_HEIGHT);

    glyph_max_extents = RECT(999, 999, 0, 0);

    FT_ULong c;
    for (c = 0; c < ARRAY_COUNT(glyphs); ++c)
    {
        u8 *buffer;
        err = heiro_load_glyph(&buffer, &glyphs[c], ft_face,
                               HEIRO_CODEPOINT_FIRST + c);
        // TODO: Use placeholder character when glyph is missing and keep going
        if (err) break;

        // Non-renderable character, no pixel data to load
        if (!buffer) continue;

        glyph_max_extents.x = 0;
        glyph_max_extents.y = MIN(glyph_max_extents.y, -glyphs[c].bearing_top);
        glyph_max_extents.w = MAX(glyph_max_extents.w, (s32)glyphs[c].width);
        glyph_max_extents.h = MAX(glyph_max_extents.h, (s32)glyphs[c].height);

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

int heiro_load_glyph(u8 **buffer, struct ric_heiro_glyph *glyph,
                     FT_Face ft_face, FT_ULong char_code)
{
    enum rico_error err = RIC_SUCCESS;
    FT_Error ft_err;

    // Note: Looks up glyph based on on current charmap
    FT_TRY(FT_Load_Char(ft_face, char_code, FT_LOAD_RENDER), RIC_ERR_FREETYPE_CHAR);
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
//void heiro_upload_glyph(struct ric_heiro_glyph *glyph, const void *pixels)
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

// TODO: Pass arena into this. Sometimes frame arena, sometimes longer lasting
//       arena for strings you don't want to build multiple times.
extern void ric_heiro_build(struct ric_heiro_string **result,
                             struct rect *cursor, const struct dlb_string *str,
                             u32 cur)
{
    s32 x = 0;
    s32 y = 0;
    float scale = 1.0f;
    const s32 glyph_h = (s32)(HEIRO_GLYPH_HEIGHT * scale);
    const s32 extent_y = (s32)(glyph_max_extents.y * scale);

    u32 len = MIN(str->len - 1, HEIRO_MAX_LEN);
    struct ric_heiro_string *string =
        calloc(1, sizeof(*string) + len * HEIRO_GLYPH_VERTS *
               sizeof(string->verts[0]));
    string->length = len;
    string->verts = (void *)((u8 *)string + sizeof(*string));

    // NOTE: Need to iterate until i == len for cursor code
    for (u32 i = 0; i <= string->length; ++i)
    {
        if (i == cur && cursor)
        {
            cursor->x = x;
            // HACK: Is there a more accurate way to calculate this?
            cursor->y = y + extent_y;
            cursor->w = 1;
            cursor->h = glyph_h;
        }
        if (i == string->length) break;

        u8 c = str->s[i];
        if (c == '\n')
        {
            x = 0;
            y += glyph_h;
            string->bounds.h = y;
            continue;
        }

        // Ignore characters we don't have glyphs for
        if (c < HEIRO_CODEPOINT_FIRST || c > HEIRO_CODEPOINT_LAST)
        {
            c = '?';
            RICO_ASSERT(c >= HEIRO_CODEPOINT_FIRST &&
                        c <= HEIRO_CODEPOINT_LAST);
        }
        struct ric_heiro_glyph *glyph = &glyphs[c - HEIRO_CODEPOINT_FIRST];

        // Glyph size
        float gx = x + (glyph->bearing_left * scale);
        float gy = y - (glyph->bearing_top * scale);
        float gw = glyph->width * scale;
        float gh = glyph->height * scale;

        float x0 = X_TO_NDC(gx);
        float y0 = Y_TO_NDC(gy);
        float x1 = X_TO_NDC(gx + gw);
        float y1 = Y_TO_NDC(gy + gh);
        float z = -1.0f;

        float u0 = glyph->uvs[0].u;
        float v0 = glyph->uvs[0].v;
        float u1 = glyph->uvs[1].u;
        float v1 = glyph->uvs[1].v;

        struct vec4 color = COLOR_WHITE;

        string->verts[string->vertex_count++] = (struct text_vertex) {
            VEC3(x0, y0, z),
            VEC2(u0, v0),
            color
        };
        string->verts[string->vertex_count++] = (struct text_vertex) {
            VEC3(x0, y1, z),
            VEC2(u0, v1),
            color
        };
        string->verts[string->vertex_count++] = (struct text_vertex) {
            VEC3(x1, y0, z),
            VEC2(u1, v0),
            color
        };
        string->verts[string->vertex_count++] = (struct text_vertex) {
            VEC3(x1, y0, z),
            VEC2(u1, v0),
            color
        };
        string->verts[string->vertex_count++] = (struct text_vertex) {
            VEC3(x0, y1, z),
            VEC2(u0, v1),
            color
        };
        string->verts[string->vertex_count++] = (struct text_vertex) {
            VEC3(x1, y1, z),
            VEC2(u1, v1),
            color
        };

        s32 adv = (glyph->advance_x >> 6);
        x += (s32)(adv * scale);
        string->bounds.w = MAX(string->bounds.w, x);
    }

    string->bounds.x = 0;
    string->bounds.y = -extent_y;
    string->bounds.h += glyph_h;
    *result = string;
}

extern void ric_heiro_render(struct ric_heiro_string *string, s32 sx, s32 sy,
                              const struct vec4 *color)
{
    struct program_text *prog = global_prog_text;
    RICO_ASSERT(prog->program.gl_id);
    glUseProgram(prog->program.gl_id);

    sx += string->bounds.x;
    sy += string->bounds.y;

    struct mat4 model = MAT4_IDENT;
    struct vec3 t_vec = VEC3(X_TO_NDC(sx) + 1.0f, Y_TO_NDC(sy) - 1.0f, 0.0f);
    mat4_translate(&model, &t_vec);

    glUniformMatrix4fv(prog->locations.vert.proj, 1, GL_TRUE,
                       cam_player.ortho_matrix.a);
    glUniformMatrix4fv(prog->locations.vert.view, 1, GL_TRUE, MAT4_IDENT.a);
    glUniformMatrix4fv(prog->locations.vert.model, 1, GL_TRUE, model.a);

    glUniform4fv(prog->locations.frag.color, 1, (const GLfloat *)color);
    glUniform1i(prog->locations.frag.grayscale, true);
    glUniform1i(prog->locations.frag.tex0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(glyph_vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glyphs[0].gl_id);

    glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    string->vertex_count * sizeof(string->verts[0]),
                    string->verts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, string->vertex_count);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

extern void ric_heiro_free(struct ric_heiro_string *string)
{
    free(string);
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

void heiro_delete_glyph(struct ric_heiro_glyph *glyph)
{
    glDeleteTextures(1, &glyph->gl_id);
}

void heiro_free()
{
    heiro_delete_glyphs();
}

// FreeType error handler
const char* getErrorMessage(FT_Error err)
{
#undef __FTERRORS_H__
#define FT_ERRORDEF(e, v, s)  case e: return s;
#define FT_ERROR_START_LIST   switch (err) {
#define FT_ERROR_END_LIST     }
#include FT_ERRORS_H
    return "Unknown error code";
}