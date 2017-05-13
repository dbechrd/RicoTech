#ifndef RICO_FONT_H
#define RICO_FONT_H

#include "geom.h"
#include "rico_mesh.h"
#include "rico_texture.h"
#include <GL/gl3w.h>

extern const u32 RICO_FONT_SIZE;

extern u32 RICO_FONT_DEFAULT;

int rico_font_init(u32 pool_size);
int font_init(const char *filename, u32 *_handle);
void font_free(u32 handle);
int font_render(u32 handle, int x, int y, struct col4 bg, const char *text,
                const char *mesh_name, enum rico_mesh_type type, u32 *_mesh,
                u32 *_texture);
#endif // RICO_FONT_H
