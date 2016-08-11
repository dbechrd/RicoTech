#ifndef SHADER_H
#define SHADER_H

#include "util.h"
#include <GL/gl3w.h>
#include <stdlib.h>
#include <stdio.h>

/*************************************************************************
| Shader types:
|
| GL_VERTEX_SHADER      Vertex shader.
| GL_GEOMETRY_SHADER    Geometry shader.
| GL_FRAGMENT_SHADER    Fragment shader.
|
*************************************************************************/
static inline GLuint make_shader(const GLenum type, const char *filename)
{
    GLint len;
    GLchar *source;
    GLuint shader;
    GLint status;

    source = file_contents(filename, &len);
    if (!source)
    {
        return 0;
    }

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &len);
    free(source);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        fprintf(stderr, "Failed to compile shader '%s':\n", filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static inline void free_shader(GLuint shader)
{
    if (shader)
    {
        glDeleteShader(shader);
    }
}

#endif // SHADER_H