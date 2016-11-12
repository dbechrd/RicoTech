#ifndef RICO_TEXTURE_H
#define RICO_TEXTURE_H

#include "rico_uid.h"
#include <GL/gl3w.h>

extern uint32 RICO_TEXTURE_DEFAULT;

int rico_texture_init(uint32 pool_size);
int texture_load_file(const char *name, GLenum target, const char *filename,
                      uint32 *_handle);
int texture_load_pixels(const char *name, GLenum target, int width, int height,
                        int bpp, const void *pixels, uint32 *_handle);
void texture_free(uint32 *handle);
const char *texture_name(uint32 handle);
void texture_bind(uint32 handle);
void texture_unbind(uint32 handle);

#endif // RICO_TEXTURE_H