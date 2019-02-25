static int make_shader(const GLenum type, const char *filename, GLuint *_shader)
{
    enum ric_error err;
    u32 len;
    GLchar *source;
    GLuint shader;
    GLint status;

    err = file_contents(filename, &source, &len);
    if (err) goto cleanup;
    RICO_ASSERT(len <= (u32)INT_MAX);

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, (GLint *)&len);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        err = RICO_ERROR(RIC_ERR_SHADER_COMPILE, "Failed to compile shader '%s'",
                         filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
    }

    *_shader = shader;

cleanup:
    free(source);
    return err;
}