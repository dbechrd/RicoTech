//#include <SFML/Window.h>
//#include <SFML/Graphics.h>
//#include <SFML/System/Types.h>
//#include <SFML/System/Clock.h>
//#include <SFML/System.h>

#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//#include <SFML/OpenGL.h>

#include "meshes.h"
#include "vec.h"
#include "util.h"
#include "structs.h"

static int make_resources(const char *vertex_shader_file);
static GLuint make_buffer(const GLenum target, const void *buffer_data, const GLsizei buffer_size);
static GLuint make_texture(const char *filename);
static GLuint make_shader(const GLenum type, const char *filename);
static GLuint make_program(const GLuint vertex_shader, const GLuint fragment_shader);
static void update_timer(const sfClock* clock);
static void render(void);
void show_info_log(
	GLuint object,
	PFNGLGETSHADERIVPROC glGet__iv,
	PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

int main(int argc, const char *argv[])
{
	//Initialize window
	sfVideoMode videoMode = {800, 600};
	sfRenderWindow *window = sfRenderWindow_create(videoMode,
		"Test Window", sfTitlebar | sfClose, NULL);

	//sfRenderWindow_setVerticalSyncEnabled(window, sfTrue);

	glewInit();

	if (!GLEW_VERSION_2_0)
	{
		char *glew = NULL;
		if (GLEW_VERSION_4_5) glew = "4.5";
		else if (GLEW_VERSION_4_4) glew = "4.4";
		else if (GLEW_VERSION_4_3) glew = "4.3";
		else if (GLEW_VERSION_4_2) glew = "4.2";
		else if (GLEW_VERSION_4_1) glew = "4.1";
		else if (GLEW_VERSION_4_0) glew = "4.0";
		else if (GLEW_VERSION_3_3) glew = "3.3";
		else if (GLEW_VERSION_3_2) glew = "3.2";
		else if (GLEW_VERSION_3_1) glew = "3.1";
		else if (GLEW_VERSION_3_0) glew = "3.0";
		else if (GLEW_VERSION_2_1) glew = "2.1";
		else if (GLEW_VERSION_2_0) glew = "2.0";
		else if (GLEW_VERSION_1_5) glew = "1.5";
		else if (GLEW_VERSION_1_4) glew = "1.4";
		else if (GLEW_VERSION_1_3) glew = "1.3";
		else if (GLEW_VERSION_1_2) glew = "1.2";
		else if (GLEW_VERSION_1_1) glew = "1.1";

		fprintf(stderr, "OpenGL 2.0 not available\n");
		fprintf(stdout, "GLEW version: %s\n", glew);

		getchar();
		return 1;
	}

	if (!make_resources(argc >= 2 ? argv[1] : "rotation.v.glsl"))
	{
		fprintf(stderr, "Failed to load resources\n");
		getchar();
		return 1;
	}

	sfClock *clock = sfClock_create();

	while(sfRenderWindow_isOpen(window))
	{
		sfEvent event;
		while (sfRenderWindow_pollEvent(window, &event))
		{
			if (event.type == sfEvtClosed)
			{
				sfRenderWindow_close(window);
			}
			else if (event.type == sfEvtResized)
			{
				glViewport(0, 0, event.size.width, event.size.height);
			}
		}

		sfColor color = sfColor_fromRGB(0, 120, 0);
		sfRenderWindow_clear(window, color);

		update_timer(clock);

		render();
		sfRenderWindow_display(window);
	}

	return 0;
}

static int make_resources(const char *vertex_shader_file)
{
	g_resources.vertex_buffer = make_buffer(
		GL_ARRAY_BUFFER,
		g_vertex_buffer_data,
		sizeof(g_vertex_buffer_data)
	);
	g_resources.element_buffer = make_buffer(
		GL_ELEMENT_ARRAY_BUFFER,
		g_element_buffer_data,
		sizeof(g_element_buffer_data)
	);

	g_resources.textures[0] = make_texture("hello1.tga");
	g_resources.textures[1] = make_texture("hello2.tga");

	if (g_resources.textures[0] == 0 || g_resources.textures[1] == 0)
		return 0;

	g_resources.vertex_shader = make_shader(
		GL_VERTEX_SHADER,
		vertex_shader_file
	);
	if (g_resources.vertex_shader == 0)
		return 0;

	g_resources.fragment_shader = make_shader(
		GL_FRAGMENT_SHADER,
		"durian.f.glsl"
	);
	if (g_resources.fragment_shader == 0)
		return 0;

	g_resources.program = make_program(
		g_resources.vertex_shader,
		g_resources.fragment_shader
	);
	if (g_resources.program == 0)
		return 0;

	int getUniformLocations = 1;

	g_resources.uniforms.timer
		= glGetUniformLocation(g_resources.program, "timer");
	if (g_resources.uniforms.timer == -1)
	{
		fprintf(stderr, "Failed to get uniform location for timer.\n");
		getUniformLocations = 0;
	}

	g_resources.uniforms.textures[0]
		= glGetUniformLocation(g_resources.program, "textures[0]");
	if (g_resources.uniforms.textures[0] == -1)
	{
		fprintf(stderr, "Failed to get uniform location for textures[0].\n");
		getUniformLocations = 0;
	}

	g_resources.uniforms.textures[1]
		= glGetUniformLocation(g_resources.program, "textures[1]");
	if (g_resources.uniforms.textures[1] == -1)
	{
		fprintf(stderr, "Failed to get uniform location for textures[1].\n");
		getUniformLocations = 0;
	}

	g_resources.attributes.position
		= glGetAttribLocation(g_resources.program, "position");
	if (g_resources.attributes.position == -1)
	{
		fprintf(stderr, "Failed to get atrribute location for position.\n");
		getUniformLocations = 0;
	}

	if (getUniformLocations == 0)
		return 0;

	return 1;
}

static GLuint make_buffer(const GLenum target, const void *buffer_data, const GLsizei buffer_size)
{
	GLuint buffer = 0;
	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
	return buffer;
}

static GLuint make_texture(const char *filename)
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,	  GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,	  GL_CLAMP_TO_EDGE);

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

static GLuint make_shader(const GLenum type, const char *filename)
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

static GLuint make_program(const GLuint vertex_shader, const GLuint fragment_shader)
{
	GLint program_ok;

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
	if (!program_ok)
	{
		fprintf(stderr, "Failed to link shader program:\n");
		show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
		glDeleteProgram(program);
		return 0;
	}

	return program;
}

static void update_timer(const sfClock *clock)
{
	float milliseconds = sfTime_asSeconds(sfClock_getElapsedTime(clock));
	g_resources.timer = (GLfloat)milliseconds; //sinf((float)milliseconds * 0.001f) * 1.0f + 1.0f;
}

static void render(void)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(g_resources.program);

	glUniform1f(g_resources.uniforms.timer, g_resources.timer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_resources.textures[0]);
	glUniform1i(g_resources.uniforms.textures[0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_resources.textures[1]);
	glUniform1i(g_resources.uniforms.textures[1], 1);

	glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
	glVertexAttribPointer(
		g_resources.attributes.position,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(GLfloat)*4,
		(void*)0
	);
	glEnableVertexAttribArray(g_resources.attributes.position);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);
	glDrawElements(
		GL_TRIANGLE_STRIP,
		4,
		GL_UNSIGNED_SHORT,
		(void*)0
	);

	glDisableVertexAttribArray(g_resources.attributes.position);
}

void show_info_log(
	GLuint object,
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
}