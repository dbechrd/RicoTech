#ifndef RICO_TEXTURE_H
#define RICO_TEXTURE_H

struct rico_texture {
    u32 id;
    u32 width;
    u32 height;
    u32 bpp;

    // TODO: Helper functions
    //const char *name;
    //u8 *pixels;
    u32 name_offset;
    u32 pixels_offset;

    // TODO: Store these in hash table when loaded, map UID -> gl id or
    //       track which textures are currently loaded on GPU some other way.
    GLuint gl_id;
    GLenum gl_target;
};

global const char *texture_name(struct rico_texture *tex);
void texture_bind(struct rico_texture *texture, GLenum texture_unit);
void texture_unbind(struct rico_texture *texture, GLenum texture_unit);

#endif // RICO_TEXTURE_H
