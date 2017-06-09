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

extern struct hnd RICO_DEFAULT_TEXTURE_DIFF;
extern struct hnd RICO_DEFAULT_TEXTURE_SPEC;

struct hnd texture_request(struct hnd handle);
int texture_request_by_name(struct hnd *_handle, enum rico_persist persist,
                            const char *name);
int texture_load_file(struct hnd *_handle, enum rico_persist persist,
                      const char *name, GLenum target, const char *filename,
                      u32 bpp);
int texture_load_pixels(struct hnd *_handle, enum rico_persist persist,
                        const char *name, GLenum target, u32 width, u32 height,
                        u32 bpp, const void *pixels);
void texture_free(struct hnd handle);
const char *texture_name(struct hnd handle);
void texture_bind(struct hnd handle, GLenum texture_unit);
void texture_unbind(struct hnd handle, GLenum texture_unit);
void texture_bind_diff(struct hnd handle);
void texture_bind_spec(struct hnd handle);
void texture_unbind_diff(struct hnd handle);
void texture_unbind_spec(struct hnd handle);

#endif // RICO_TEXTURE_H