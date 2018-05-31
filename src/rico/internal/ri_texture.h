#ifndef RICO_INTERNAL_TEXTURE_H
#define RICO_INTERNAL_TEXTURE_H

struct rgl_texture
{
    GLuint gl_id;
    GLenum gl_target;
    GLenum format_internal;
    GLenum format;
    struct rgl_texture *next;
};

struct RICO_texture
{
    struct uid uid;
    u32 width;
    u32 height;
    u8 bpp;
    GLenum gl_target;

    u32 pixels_offset;
};

extern void rico_texture_init();
static void texture_delete(struct RICO_texture *texture);
static void texture_bind(pkid pkid, GLenum texture_unit);
static void texture_unbind(pkid pkid, GLenum texture_unit);

#endif