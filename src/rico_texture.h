#ifndef RICO_TEXTURE_H
#define RICO_TEXTURE_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_texture {
    struct hnd hnd;

    // TODO: Save filename (is this hnd->name? might get truncated.. we need
    //                      the whole thing for lookups on load, right?)
    // TODO: Clear gl_id before saving to file
    GLuint gl_id;
    GLenum gl_target;

    //TODO: What's the point of saving this if we don't also save pixel data?
    GLsizei width;
    GLsizei height;
    GLsizei bpp;
};
extern struct rico_texture *RICO_DEFAULT_TEXTURE_DIFF;
extern struct rico_texture *RICO_DEFAULT_TEXTURE_SPEC;

int texture_load_file(struct rico_texture *texture, const char *name,
                      GLenum target, const char *filename, u32 bpp);
int texture_load_pixels(struct rico_texture *texture, const char *name,
                        GLenum target, u32 width, u32 height, u32 bpp,
                        const void *pixels);
int texture_free(struct rico_texture *texture);
const char *texture_name(struct rico_texture *texture);
void texture_bind(struct rico_texture *texture, GLenum texture_unit);
void texture_unbind(struct rico_texture *texture, GLenum texture_unit);

#endif // RICO_TEXTURE_H
