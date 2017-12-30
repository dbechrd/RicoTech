#ifndef RICO_FONT_H
#define RICO_FONT_H

// HACK: Hard-coded font width/height
// TODO: Figure out how to determine font w/h in screen space
#define FONT_WIDTH 12.5f
#define FONT_HEIGHT 25.0f

#define BFG_MAXSTRING 511  // Maximum string length

struct rico_font
{
    u32 id;
    u32 cell_x;
    u32 cell_y;
    u8 base_char;
    u32 row_pitch;
    float col_factor;
    float row_factor;
    s32 y_offset;
    bool y_invert;
    u8 render_style;
    u8 char_widths[256];
    u32 texture_id;

    u32 name_offset;
};

global const char *font_name(struct rico_font *font);
void font_render(u32 *mesh_id, u32 *texture_id, struct rico_font *font, int x,
                 int y, struct vec4 bg, const char *text,
                 const char *mesh_name);

#endif // RICO_FONT_H
