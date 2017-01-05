#ifndef RICO_TEXTURE_H
#define RICO_TEXTURE_H

#include "rico_uid.h"
#include "rico_pool.h"
#include <GL/gl3w.h>

extern u32 RICO_TEXTURE_DEFAULT_DIFF;
extern u32 RICO_TEXTURE_DEFAULT_SPEC;

int rico_texture_init(u32 pool_size);
int texture_request(u32 handle);
int texture_load_file(const char *name, GLenum target, const char *filename,
                      u32 *_handle);
int texture_load_pixels(const char *name, GLenum target, int width, int height,
                        int bpp, const void *pixels, u32 *_handle);
void texture_free(u32 handle);
const char *texture_name(u32 handle);
void texture_bind(u32 handle, GLenum texture_unit);
void texture_unbind(u32 handle, GLenum texture_unit);

struct rico_pool *texture_pool_unsafe();

#endif // RICO_TEXTURE_H