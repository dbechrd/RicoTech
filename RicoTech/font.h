#ifndef FONT_H
#define FONT_H

#include "geom.h"
#include <GL\gl3w.h>
#include <stdbool.h>

struct font {
    int CellX, CellY, YOffset, RowPitch;
    char Base;
    char Width[256];
    GLuint TexID;
    int CurX, CurY;
    float RowFactor, ColFactor;
    int RenderStyle;
    bool InvertYAxis;
};

struct font *make_font(const char *filename);
void font_render(const struct font *font, const char *text, struct col4 bg);

#endif // FONT_H
