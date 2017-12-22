#ifndef RICO_TEXTURE_H
#define RICO_TEXTURE_H

struct rico_texture {
    struct hnd hnd;
    u32 width;
    u32 height;
    u32 bpp;
    u8 *pixels;

    // TODO: Store these in hash table when loaded, map UID -> gl id or
    //       track which texture is currently loaded on GPU some other way.
    GLuint gl_id;
    GLenum gl_target;
};
extern struct pool_id RICO_DEFAULT_TEXTURE_DIFF;
extern struct pool_id RICO_DEFAULT_TEXTURE_SPEC;

int texture_load_file(struct rico_texture *texture, const char *name,
                      GLenum target, const char *filename, u32 bpp);
int texture_load_pixels(struct rico_texture *texture, const char *name,
                        GLenum target, u32 width, u32 height, u32 bpp,
                        const void *pixels);
int texture_free(struct rico_texture *texture);
void texture_bind(struct rico_texture *texture, GLenum texture_unit);
void texture_unbind(struct rico_texture *texture, GLenum texture_unit);

#endif // RICO_TEXTURE_H
