#include "shader.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <stdlib.h>

GLuint make_shader(const GLenum type, const char *filename)
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