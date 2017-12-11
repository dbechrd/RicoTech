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

    uid texture_uid;
    struct rico_texture *texture;
};
extern struct rico_font *RICO_DEFAULT_FONT;

inline void font_fixup(struct rico_font *font)
{
    font->texture = hashtable_search_uid(&global_uids, font->texture_uid);
}

int font_init(struct rico_font *font, const char *filename);
void font_free(struct rico_font *font);
int font_render(struct rico_mesh **_mesh, struct rico_texture **_texture,
                struct rico_font *font, int x, int y, struct col4 bg,
                const char *text, const char *mesh_name,
                enum rico_mesh_type type);

#endif // RICO_FONT_H
