#ifndef RICO_FONT_H
#define RICO_FONT_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_font {
    struct rico_uid uid;
    int CellX, CellY, YOffset, RowPitch;
    char Base;
    char Width[256];

    // TODO: Use rico_texture??
    u32 texture;
    float RowFactor, ColFactor;
    int RenderStyle;
    bool InvertYAxis;
};
extern const u32 RICO_FONT_SIZE;

extern u32 RICO_DEFAULT_FONT;

int font_init(const char *filename, u32 *_handle);
void font_free(u32 handle);
int font_render(u32 handle, int x, int y, struct col4 bg, const char *text,
                const char *mesh_name, enum rico_mesh_type type, u32 *_mesh,
                u32 *_texture);
#endif // RICO_FONT_H
