#ifndef RICO_FONT_H
#define RICO_FONT_H

// HACK: Hard-coded font width/height
// TODO: Figure out how to determine font w/h in screen space
#define FONT_WIDTH 12.5f
#define FONT_HEIGHT 25.0f

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_font {
    struct hnd hnd;
    int CellX, CellY, YOffset, RowPitch;
    char Base;
    char Width[256];
    float RowFactor, ColFactor;
    int RenderStyle;
    bool InvertYAxis;

    struct pool_id texture_id;
};
extern struct pool_id RICO_DEFAULT_FONT;

int font_init(struct rico_font *font, const char *filename);
int font_free(struct rico_font *font);
int font_render(struct pool_id *mesh_id, struct pool_id *texture_id,
                struct rico_font *font, int x, int y, struct col4 bg,
                const char *text, const char *mesh_name,
                enum rico_mesh_type type);

#endif // RICO_FONT_H
