#ifndef RICO_FONT_H
#define RICO_FONT_H

#define WIDTH_DATA_OFFSET  20  // Offset to width data with BFF file
#define MAP_DATA_OFFSET   276  // Offset to texture image data with BFF file

#define BFG_RS_NONE  0x0  // Blend flags
#define BFG_RS_ALPHA 0x1
#define BFG_RS_RGB   0x2
#define BFG_RS_RGBA  0x4

// HACK: Hard-coded font width/height
// TODO: Figure out how to determine font w/h in screen space
#define FONT_WIDTH 8.0f
#define FONT_HEIGHT 16.0f

#define BFG_MAXSTRING 511  // Maximum string length

struct rico_font
{
    struct uid uid;
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
    pkid tex_pkid;
};

void font_render(u32 *mesh_id, u32 *tex_id, struct rico_font *font, float x,
                 float y, struct vec4 bg, const char *text,
                 const char *mesh_name);
#endif