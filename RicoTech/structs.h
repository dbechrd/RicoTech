#ifndef STRUCTS_H
#define STRUCTS_H

struct {
    enum {
        VBO_POSITION,
        VBO_POSITION_ELEM,
        NUM_BUFFERS
    } buffer_enum;

    GLuint vao;
    GLuint vbos[NUM_BUFFERS];
    GLuint textures[2];
    GLuint vertex_shader, fragment_shader, program;

    struct {
        GLint timer;
        GLint textures[2];
    } uniforms;

    struct {
        GLint position;
    } attributes;

    GLfloat timer;
} g_resources;

static const struct Vec4 g_vertices[] = {
    {-1.0f, -1.0f, 0.0f, 1.0f },
    { 1.0f, -1.0f, 0.0f, 1.0f },
    {-1.0f,  1.0f, 0.0f, 1.0f },
    { 1.0f,  1.0f, 0.0f, 1.0f }
};

//static const GLfloat g_vertex_buffer_data[] = {
//    -1.0f, -1.0f, 0.0f, 1.0f,
//     1.0f, -1.0f, 0.0f, 1.0f,
//    -1.0f,  1.0f, 0.0f, 1.0f,
//     1.0f,  1.0f, 0.0f, 1.0f
//};

static const GLushort g_indices[] = { 0, 1, 2, 3 };

#endif