#include "shader.h"
#include "const.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <stdlib.h>

int make_shader(const GLenum type, const char *filename, GLuint *_shader)
{
    enum rico_error err;
    GLint len;
    GLchar *source;
    GLuint shader;
    GLint status;

    err = file_contents(filename, &len, &source);
    if (err) return err;

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
        return RICO_ERROR(ERR_SHADER_COMPILE);
    }

    *_shader = shader;
    return SUCCESS;
}