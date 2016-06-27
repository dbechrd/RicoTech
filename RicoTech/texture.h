#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/gl3w.h>

struct texture {
    GLuint texture;
    GLenum target;

    GLsizei width;
    GLsizei height;
    GLsizei bpp;
};

struct texture *make_texture(GLenum target, const char *filename);
void free_texture(struct texture **tex);

static inline bind_texture(const struct texture *tex)
{
    glBindTexture(GL_TEXTURE_2D, tex->texture);
}

#endif // TEXTURE_H