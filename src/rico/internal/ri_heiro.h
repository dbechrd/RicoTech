#ifndef RICO_HEIRO_H
#define RICO_HEIRO_H

struct RICO_heiro_glyph
{
    s32 width;
    s32 height;
    s32 bearing_left;
    s32 bearing_top;
    u32 advance;

    u32 tex_pkid;
};

int rico_heiro_init();
int heiro_load_glyphs();
int heiro_load_glyph(struct RICO_heiro_glyph *glyph, FT_ULong char_code);
void heiro_free();

#endif