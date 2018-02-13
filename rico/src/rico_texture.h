#ifndef RICO_TEXTURE_H
#define RICO_TEXTURE_H

// NOTE: Must fit in hash value
struct rgl_texture
{
    GLenum gl_target;
    GLuint gl_id;
    GLenum format_internal;
    GLenum format;
};

struct rico_texture
{
    struct uid uid;
    u32 width;
    u32 height;
    u8 bpp;
    GLenum gl_target;

    u32 pixels_offset;
};

void texture_delete(struct rico_texture *texture);
void texture_bind(pkid pkid, GLenum texture_unit);
void texture_unbind(pkid pkid, GLenum texture_unit);

#endif