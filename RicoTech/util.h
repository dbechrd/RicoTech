#ifndef UTIL_H
#define UTIL_H

#include <GL/gl3w.h>

void *file_contents(const char *filename, GLint *length);
void *read_tga(const char *filename, int *width, int *height);

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *message,
                                     const void *userParam);

void show_info_log(GLuint object,
                   PFNGLGETSHADERIVPROC glGet__iv,
                   PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

GLuint orig_make_texture(const char *filename);
GLuint orig_make_shader(GLenum type, const char *filename);
GLuint orig_make_program(const char *vertex_shader_filename,
                    const char *fragment_shader_filename);

#endif