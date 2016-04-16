#ifndef UTIL_H
#define UTIL_H

#include <GL/gl3w.h>

void *file_contents(const char *filename, int *length);
void *read_tga(const char *filename, int *width, int *height);

void show_info_log(GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

GLuint make_texture(const char *filename);
GLuint make_shader(const GLenum type, const char *filename);
GLuint make_program(const GLuint vertex_shader, const GLuint fragment_shader);

#endif