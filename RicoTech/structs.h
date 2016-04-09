#ifndef STRUCTS_H
#define STRUCTS_H

static struct {
	GLuint vertex_buffer, element_buffer;
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

static const GLfloat g_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f,
	-1.0f,  1.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 0.0f, 1.0f
};

static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };

#endif