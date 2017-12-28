#ifndef RICO_TEXTURE_H
#define RICO_TEXTURE_H

// NOTE: Must fit in hash value
struct rgl_texture {
    GLenum gl_target;
    GLuint gl_id;
    GLenum format_internal;
    GLenum format;
};

struct rico_texture {
    u32 id;
    u32 width;
    u32 height;
    u8 bpp;
    GLenum gl_target;

    u32 name_offset;
    u32 pixels_offset;
};

global const char *texture_name(struct rico_texture *tex);
void texture_delete(struct rico_texture *texture);
void texture_bind(struct pack *pack, u32 id, GLenum texture_unit);
void texture_unbind(struct pack *pack, u32 id, GLenum texture_unit);

#endif // RICO_TEXTURE_H
