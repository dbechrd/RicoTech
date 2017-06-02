int make_shader(const GLenum type, const char *filename, GLuint *_shader)
{
    enum rico_error err;
    GLint len;
    GLchar *source;
    GLuint shader;
    GLint status;

    err = file_contents(filename, &len, &source);
    if (err) goto cleanup;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &len);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        err = RICO_ERROR(ERR_SHADER_COMPILE, "Failed to compile shader '%s'",
                         filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
    }

    *_shader = shader;

cleanup:
    free(source);
    return err;
}