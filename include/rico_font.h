#ifndef RICO_FONT_H
#define RICO_FONT_H

#include "geom.h"
#include "rico_mesh.h"
#include "rico_texture.h"
#include <GL/gl3w.h>
#include <stdbool.h>

struct rico_font {
    int CellX, CellY, YOffset, RowPitch;
    char Base;
    char Width[256];

    // TODO: Use rico_texture??
    uint32 texture;
    float RowFactor, ColFactor;
    int RenderStyle;
    bool InvertYAxis;
};

struct rico_font *make_font(const char *filename);
int font_render(const struct rico_font *font, int x, int y, const char *text,
                struct col4 bg, uint32 *_mesh, uint32 *_texture);
#endif // RICO_FONT_H
