// Boring, non-OpenGL-related utility functions

int file_contents(const char *filename, int *_length, char **_buffer)
{
    FILE *fs = fopen(filename, "rb");

    if (!fs) {
        return RICO_ERROR(ERR_FILE_READ, "Unable to open %s for reading",
                          filename);
    }

    fseek(fs, 0, SEEK_END);
    *_length = ftell(fs);
    fseek(fs, 0, SEEK_SET);

    if (!*_length) {
        return RICO_ERROR(ERR_FILE_READ, "Unable to determine length of %s",
                          filename);
    }

    *_buffer = malloc(*_length + 1 * sizeof(char));
    if (!*_buffer) {
        return RICO_ERROR(ERR_BAD_ALLOC,
                          "Failed to allocate file buffer for %s", filename);
    }

    *_length = fread(*_buffer, 1, *_length, fs);
    fclose(fs);
    (*_buffer)[*_length] = '\0';

    return SUCCESS;
}

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *message,
                                     const void *userParam)
{
    //HACK: Get rid of warning-as-error for unused parameters
    (void)length;
    (void)userParam;

    char *sourceStr, *typeStr, *severityStr;

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

    fprintf(stderr, "[%s][%s][%s][%d] %s\n", sourceStr, typeStr, severityStr,
            id, message);
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

////////////////////////////////////////////////////////////////////////////////

/*
internal short le_short(unsigned char *bytes)
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
        fprintf(stderr, "%s is not a 24-bit uncompressed RGB tga file\n",
                filename);
        fclose(f);
        return NULL;
    }

    for (i = 0; i < header.id_length; ++i)
        if (getc(f) == EOF) {
            fprintf(stderr, "%s has incomplete id string\n", filename);
            fclose(f);
            return NULL;
        }

    color_map_size = le_short(header.color_map_length) *
                     (header.color_map_depth / 8);
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
*/