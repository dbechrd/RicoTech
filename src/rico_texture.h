#ifndef RICO_TEXTURE_H
#define RICO_TEXTURE_H

struct rico_texture {
    u32 id;
    u32 width;
    u32 height;
    u32 bpp;
    GLenum gl_target;

    u32 name_offset;
    u32 pixels_offset;

    // TODO: Store these in hash table when loaded, map UID -> gl id or
    //       track which textures are currently loaded on GPU some other way.
    bool loaded;
    GLuint gl_id;
};

global const char *texture_name(struct rico_texture *tex);
void texture_bind(struct pack *pack, u32 id, GLenum texture_unit);
void texture_unbind(struct pack *pack, u32 id, GLenum texture_unit);

#endif // RICO_TEXTURE_H
