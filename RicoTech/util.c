#include "util.h"

//#include <math.h>

#include <stdio.h>
#include <stdlib.h>

/*
* Boring, non-OpenGL-related utility functions
*/

void *file_contents(const char *filename, GLint *length)
{
    FILE *fs = fopen(filename, "r");
    void *buffer;

    if (!fs) {
        fprintf(stderr, "Unable to open %s for reading\n", filename);
        return NULL;
    }

    fseek(fs, 0, SEEK_END);
    *length = ftell(fs);
    fseek(fs, 0, SEEK_SET);

    buffer = malloc(*length + 1);
    *length = fread(buffer, 1, *length, fs);
    fclose(fs);
    ((char*)buffer)[*length] = '\0';

    return buffer;
}

static short le_short(unsigned char *bytes)
{
    return bytes[0] | ((char)bytes[1] << 8);
}

void *read_tga(const char *filename, int *width, int *height)
{
    struct tga_header {
        unsigned char  id_length;
        char  color_map_type;
        char  data_type_code;
        unsigned char  color_map_origin[2];
        unsigned char  color_map_length[2];
        char  color_map_depth;
        unsigned char  x_origin[2];
        unsigned char  y_origin[2];
        unsigned char  width[2];
        unsigned char  height[2];
        char  bits_per_pixel;
        char  image_descriptor;
    } header;
    size_t i, color_map_size, pixels_size;
    FILE *f;
    size_t read;
    void *pixels;

    f = fopen(filename, "rb");

    if (!f) {
        fprintf(stderr, "Unable to open %s for reading\n", filename);
        return NULL;
    }

    read = fread(&header, 1, sizeof(header), f);

    if (read != sizeof(header)) {
        fprintf(stderr, "%s has incomplete tga header\n", filename);
        fclose(f);
        return NULL;
    }
    if (header.data_type_code != 2) {
        fprintf(stderr, "%s is not an uncompressed RGB tga file\n", filename);
        fclose(f);
        return NULL;
    }
    if (header.bits_per_pixel != 24) {
        fprintf(stderr, "%s is not a 24-bit uncompressed RGB tga file\n", filename);
        fclose(f);
        return NULL;
    }

    for (i = 0; i < header.id_length; ++i)
        if (getc(f) == EOF) {
            fprintf(stderr, "%s has incomplete id string\n", filename);
            fclose(f);
            return NULL;
        }

    color_map_size = le_short(header.color_map_length) * (header.color_map_depth / 8);
    for (i = 0; i < color_map_size; ++i)
        if (getc(f) == EOF) {
            fprintf(stderr, "%s has incomplete color map\n", filename);
            fclose(f);
            return NULL;
        }

    *width = le_short(header.width); *height = le_short(header.height);
    pixels_size = *width * *height * (header.bits_per_pixel / 8);
    pixels = malloc(pixels_size);

    read = fread(pixels, 1, pixels_size, f);
    fclose(f);

    if (read != pixels_size) {
        fprintf(stderr, "%s has incomplete image\n", filename);
        free(pixels);
        return NULL;
    }

    return pixels;
}

////////////////////////////////////////////////////////////////////////////////

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *message,
                                     const void *userParam)
{
    //HACK: Get rid of warning-as-error for unused parameters
    (void)length;
    (void)userParam;

    char *typeStr, *sourceStr, *severityStr;

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        typeStr = "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeStr = "DEPRC";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeStr = "UNDEF";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        typeStr = "PORT ";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        typeStr = "PERF ";
        break;
    case GL_DEBUG_TYPE_OTHER:
        typeStr = "OTHER";
        return;
        break;
    default:
        typeStr = "?????";
        break;
    }

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        sourceStr = "API            ";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sourceStr = "WINDOW SYSTEM  ";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sourceStr = "SHADER COMPILER";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sourceStr = "THIRD PARTY    ";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        sourceStr = "APPLICATION    ";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        sourceStr = "OTHER          ";
        return;
        break;
    default:
        sourceStr = "???????????????";
        break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        severityStr = "LOW ";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severityStr = "MED ";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        severityStr = "HIGH";
        break;
    default:
        severityStr = "????";
        break;
    }

    fprintf(stderr, "[%s][%s][%d] %s\n", typeStr, severityStr, id, message);
}

void show_info_log(GLuint object,
                   PFNGLGETSHADERIVPROC glGet__iv,
                   PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
};

GLuint orig_make_texture(const char *filename)
{
    GLuint texture;
    int width, height;
    void *pixels = read_tga(filename, &width, &height);

    if (!pixels)
    {
        return 0;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(
        GL_TEXTURE_2D, 0,
        GL_RGB8,
        width, height, 0,
        GL_BGR, GL_UNSIGNED_BYTE,
        pixels
        );

    free(pixels);
    return texture;
}

GLuint orig_make_shader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source = file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;

    if (!source)
    {
        return 0;
    }

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    source = NULL;
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok)
    {
        fprintf(stderr, "Failed to compile %s:\n", filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint orig_make_program(const char *vertex_shader_filename,
                    const char *fragment_shader_filename)
{
    GLint program_ok;

    GLuint vertex_shader = orig_make_shader(GL_VERTEX_SHADER,
                                       vertex_shader_filename);
    if (!vertex_shader)
    {
        fprintf(stderr, "Failed to make vertex shader.\n");
        return 0;
    }

    GLuint fragment_shader = orig_make_shader(GL_FRAGMENT_SHADER,
                                         fragment_shader_filename);
    if (!fragment_shader)
    {
        glDeleteShader(vertex_shader);

        fprintf(stderr, "Rect: Failed to make fragment shader.\n");
        return 0;
    }

    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader);
    glAttachShader(program_id, fragment_shader);
    glLinkProgram(program_id);

    glDetachShader(program_id, vertex_shader);
    glDetachShader(program_id, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glGetProgramiv(program_id, GL_LINK_STATUS, &program_ok);
    if (!program_ok)
    {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program_id, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program_id);
        return 0;
    }

    return program_id;
}