#ifndef RICO_INTERNAL_SHADER_H
#define RICO_INTERNAL_SHADER_H

//#include "util.h"
//#include <GL/gl3w.h>

/*************************************************************************
| Shader types:
|
| GL_VERTEX_SHADER      Vertex shader.
| GL_GEOMETRY_SHADER    Geometry shader.
| GL_FRAGMENT_SHADER    Fragment shader.
|
*************************************************************************/
int make_shader(const GLenum type, const char *filename, GLuint *_shader);

static inline void free_shader(GLuint shader)
{
    if (shader) glDeleteShader(shader);
}

#endif