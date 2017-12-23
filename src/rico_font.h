#ifndef RICO_FONT_H
#define RICO_FONT_H

// HACK: Hard-coded font width/height
// TODO: Figure out how to determine font w/h in screen space
#define FONT_WIDTH 12.5f
#define FONT_HEIGHT 25.0f

#define BFG_MAXSTRING 511  // Maximum string length

struct rico_font {
    u32 id;
    int CellX, CellY, YOffset, RowPitch;
    char Base;
    char Width[256];
    float RowFactor, ColFactor;
    int RenderStyle;
    bool InvertYAxis;

    u32 name_offset;
    u32 texture_offset;
};
extern struct pool_id RICO_DEFAULT_FONT;

global const char *font_name(struct rico_font *font);
void font_render(struct rico_mesh **mesh, struct rico_texture **texture,
                 struct rico_font *font, int x, int y, struct col4 bg,
                 const char *text, const char *mesh_name,
                 enum rico_mesh_type type);

#endif // RICO_FONT_H
