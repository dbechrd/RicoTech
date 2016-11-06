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
    struct rico_texture *tex;
    float RowFactor, ColFactor;
    int RenderStyle;
    bool InvertYAxis;
};

struct rico_font *make_font(const char *filename);
void font_render(const struct rico_font *font, int x, int y,
                 const char *text, struct col4 bg,
                 struct rico_mesh **out_mesh,
                 struct rico_texture **out_texture);

static inline void font_bind(const struct rico_font *font)
{
    glBindTexture(GL_TEXTURE_2D, font->tex->texture_id);
}

static inline void font_unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

#endif // RICO_FONT_H
