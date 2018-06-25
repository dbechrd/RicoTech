static FT_Library ft_lib;
static FT_Face ft_face;

// TODO: Use hash table instead
static struct RICO_heiro_glyph glyphs[128];
//static struct dlb_hash glyphs;

#define FT_TRY(expr, code) \
    ft_err = (expr); \
    if (ft_err) { \
        return RICO_ERROR(code, "%d %s", ft_err, getErrorMessage(ft_err)); \
    }

const char* getErrorMessage(FT_Error err);

int rico_heiro_init()
{
    enum rico_error err;
    FT_Error ft_err;

    FT_TRY(FT_Init_FreeType(&ft_lib), ERR_FREETYPE_INIT);

    FT_TRY(FT_New_Face(ft_lib, "font/Cousine-Regular.ttf", 0, &ft_face),
           ERR_FREETYPE_FACE);
    //FREETYPE_TRY(FT_New_Memory_Face(ft_lib, buffer, buffer_size, 0, &ft_face),
    //             ERR_FREETYPE_FACE;

    // Note: Doesn't work with FT cache API
    FT_TRY(FT_Set_Pixel_Sizes(ft_face, 0, 48), ERR_FREETYPE_SIZE);

    // TODO: Do I need to set cmap or encoding?
    //FT_Set_Charmap(ft_face, charmap);
    //FT_Select_Charmap(ft_face, encoding);

    err = heiro_load_glyphs();
    if (err) return err;

    return SUCCESS;
}

int heiro_load_glyphs()
{
    enum rico_error err;

    for (FT_ULong c = 0; c < ARRAY_COUNT(glyphs); ++c)
    {
        err = heiro_load_glyph(&glyphs[c], c);
        if (err) return err;
    }

    return SUCCESS;
}

int heiro_load_glyph(struct RICO_heiro_glyph *glyph, FT_ULong char_code)
{
    FT_Error ft_err;

    // Note: Looks up glyph based on on current charmap
    FT_TRY(FT_Load_Char(ft_face, char_code, FT_LOAD_RENDER), ERR_FREETYPE_CHAR);
    //FT_TRY(FT_Load_Glyph(ft_face, glyph_code, FT_LOAD_RENDER);

    glyph->width        = ft_face->glyph->bitmap.width;
    glyph->height       = ft_face->glyph->bitmap.rows;
    glyph->bearing_left = ft_face->glyph->bitmap_left;
    glyph->bearing_top  = ft_face->glyph->bitmap_top;
    glyph->advance      = ft_face->glyph->advance.x;

    return SUCCESS;
}

void heiro_free()
{
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);
}

const char* getErrorMessage(FT_Error err)
{
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
    return "(Unknown error)";
}