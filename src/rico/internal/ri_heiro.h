#ifndef RI_HEIRO_H
#define RI_HEIRO_H

#define HEIRO_GLYPH_HEIGHT 16

struct RICO_heiro_glyph
{
    u32 width;
    u32 height;
    s32 bearing_left;
    s32 bearing_top;
    s32 advance_x;
    s32 advance_y;

    GLuint gl_id;
    struct vec2 uvs[2];
};

int rico_heiro_init();
int heiro_load_glyphs(FT_Face ft_face);
int heiro_load_glyph(u8 **buffer, struct RICO_heiro_glyph *glyph,
                     FT_Face ft_face, FT_ULong char_code);
GLuint heiro_upload_texture(s32 width, s32 height, const void *pixels);
void heiro_delete_glyphs();
void heiro_delete_glyph(struct RICO_heiro_glyph *glyph);
void heiro_free();

#endif