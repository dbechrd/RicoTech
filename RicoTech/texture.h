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

struct rico_texture *make_texture(GLenum target, const char *filename);
void free_texture(struct rico_texture **tex);

static inline texture_bind(const struct rico_texture *tex)
{
    glBindTexture(tex->target, tex->texture_id);
}

static inline texture_unbind(const struct rico_texture *tex)
{
    glBindTexture(tex->target, 0);
}

#endif // TEXTURE_H