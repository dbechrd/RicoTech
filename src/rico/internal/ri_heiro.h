#ifndef RI_HEIRO_H
#define RI_HEIRO_H

struct RICO_heiro_glyph
{
    s32 width;
    s32 height;
    s32 bearing_left;
    s32 bearing_top;
    u32 advance;

    u32 gl_id;
};

int rico_heiro_init();
int heiro_load_glyphs(FT_Face ft_face);
int heiro_load_glyph(struct RICO_heiro_glyph *glyph, FT_Face ft_face,
                     FT_ULong char_code);
void heiro_upload_glyph(struct RICO_heiro_glyph *glyph, const void *pixels);
void heiro_delete_glyphs();
void heiro_delete_glyph(struct RICO_heiro_glyph *glyph);
void heiro_free();

#endif