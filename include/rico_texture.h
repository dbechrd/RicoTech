#ifndef RICO_TEXTURE_H
#define RICO_TEXTURE_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_texture {
    struct rico_uid uid;
    u32 ref_count;

    GLuint gl_id;
    GLenum gl_target;

    //TODO: What's the point of saving this if we don't also save pixel data?
    GLsizei width;
    GLsizei height;
    GLsizei bpp;
};
extern const u32 RICO_TEXTURE_SIZE;

extern u32 RICO_DEFAULT_TEXTURE_DIFF;
extern u32 RICO_DEFAULT_TEXTURE_SPEC;

u32 texture_request(enum rico_persist persist, u32 handle);
int texture_request_by_name(u32 *_handle, enum rico_persist persist,
                            const char *name);
int texture_load_file(u32 *_handle, enum rico_persist persist, const char *name,
                      GLenum target, const char *filename, u32 bpp);
int texture_load_pixels(u32 *_handle, enum rico_persist persist,
                        const char *name, GLenum target, u32 width, u32 height,
                        u32 bpp, const void *pixels);
void texture_free(enum rico_persist persist, u32 handle);
const char *texture_name(enum rico_persist persist, u32 handle);
void texture_bind(enum rico_persist persist, u32 handle, GLenum texture_unit);
void texture_unbind(enum rico_persist persist, u32 handle, GLenum texture_unit);

#endif // RICO_TEXTURE_H