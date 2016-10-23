#ifndef UTIL_H
#define UTIL_H

#include <GL/gl3w.h>

void *file_contents(const char *filename, int *length);
//void *read_tga(const char *filename, int *width, int *height);

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *message,
                                     const void *userParam);

void show_info_log(GLuint object,
                   PFNGLGETSHADERIVPROC glGet__iv,
                   PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

inline unsigned short swap_16bit(unsigned short us)
{
    return (unsigned short)(((us & 0xFF00) >> 8) |
                            ((us & 0x00FF) << 8));
}

inline unsigned long swap_32bit(unsigned long ul)
{
    return (unsigned long)(((ul & 0xFF000000) >> 24) |
                           ((ul & 0x00FF0000) >>  8) |
                           ((ul & 0x0000FF00) <<  8) |
                           ((ul & 0x000000FF) << 24));
}

#endif