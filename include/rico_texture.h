#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/gl3w.h>

struct rico_texture {
    GLuint texture_id;
    GLenum target;

    GLsizei width;
    GLsizei height;
    GLsizei bpp;
};

extern struct rico_texture *RICO_TEXTURE_DEFAULT;

struct rico_texture *make_texture(GLenum target, const char *filename);
void free_texture(struct rico_texture **tex);

static inline void texture_bind(const struct rico_texture *tex)
{
    glBindTexture(tex->target, tex->texture_id);
}

static inline void texture_unbind(const struct rico_texture *tex)
{
    glBindTexture(tex->target, 0);
}

#endif // TEXTURE_H