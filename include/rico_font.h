#ifndef RICO_FONT_H
#define RICO_FONT_H

// HACK: Hard-coded font width/height
// TODO: Figure out how to determine font w/h in screen space
#define FONT_WIDTH 12.5f
#define FONT_HEIGHT 25.0f

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_font {
    struct rico_uid uid;
    int CellX, CellY, YOffset, RowPitch;
    char Base;
    char Width[256];
    float RowFactor, ColFactor;
    int RenderStyle;
    bool InvertYAxis;

    u32 texture;
};
extern const u32 RICO_FONT_SIZE;

extern u32 RICO_DEFAULT_FONT;

int font_init(u32 *_handle, enum rico_persist persist, const char *filename);
void font_free(enum rico_persist persist, u32 handle);
int font_render(u32 *_mesh, u32 *_texture, enum rico_persist persist,
                u32 handle, int x, int y, struct col4 bg, const char *text,
                const char *mesh_name, enum rico_mesh_type type);

#endif // RICO_FONT_H