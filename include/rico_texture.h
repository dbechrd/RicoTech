#ifndef TEXTURE_H
#define TEXTURE_H

#include "rico_uid.h"
#include <GL/gl3w.h>

#define RICO_TEXTURE_DEFAULT 0

int texture_set_default(GLenum target, const char *filename);
int make_texture_file(GLenum target, const char *filename, uint32 *_handle);
int make_texture_pixels(GLenum target, int width, int height, int bpp,
                        const void *pixels, uint32 *_handle);
void free_texture(uint32 *handle);
void texture_bind(uint32 handle);
void texture_unbind(uint32 handle);

#endif // TEXTURE_H