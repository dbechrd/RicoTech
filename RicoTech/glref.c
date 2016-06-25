#include "glref.h"
#include "geom.h"
#include "util.h"
#include "shader.h"
#include "program.h"
#include "stb_image.h"

#include <GL/gl3w.h>
#include <SDL/SDL_assert.h>
#include <stdio.h>
#include <malloc.h>

struct vec4 view_scale = { 1.0f, 1.0f, 1.0f };
struct vec4 view_trans = { 0.0f,-1.7f, 0.0f }; //Player's eyes are at 1.7 meters
struct vec4 view_rot = { 0.0f, 0.0f, 0.0f };

//TODO: Implement better camera with position + lookat. Is that necessary?
//      Maybe it's easy to derive lookat when I need it? Probably not..
//struct vec4 camera_right = {  }

static GLuint vao;
static GLuint vbos[2];
static GLuint shaders[2];
static GLuint program_id;
static GLuint tex_r, tex_g;

static GLint uniform_time;
static GLint uniform_model;
static GLint uniform_view;
static GLint uniform_proj;
static GLint uniform_tex;

enum { VBO_VERTEX, VBO_ELEMENT };

struct vertex {
    struct vec4 pos;
    struct col4 col;
    struct tex2 tex;
};

void init_glref()
{
    //--------------------------------------------------------------------------
    // Generate and bind VAOs
    //--------------------------------------------------------------------------
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //--------------------------------------------------------------------------
    // Generate buffer objects
    //--------------------------------------------------------------------------
    glGenBuffers(2, vbos);

    //--------------------------------------------------------------------------
    // Bind vertex data to buffer
    //--------------------------------------------------------------------------

    // Bind vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBO_VERTEX]);

    /*************************************************************************
    | Frequency of access:
    | 
    | STREAM  Data store contents modified once and used at most a few times.
    | STATIC  Data store contents modified once and used many times.
    | DYNAMIC Data store contents modified repeatedly and used many times.
    |
    **************************************************************************
    | Nature of access:
    | 
    | DRAW    The data store contents are modified by the application, and used
    |         as the source for GL drawing and image specification commands.
    | READ    The data store contents are modified by reading data from the GL,
    |         and used to return that data when queried by the application.
    | COPY    DRAW & READ
    |
    *************************************************************************/
    struct vertex vertices[4] = {
        {
            { -1.0f, -1.0f, 0.0f, 1.0f }, //Position
            { 1.0f, 1.0f, 1.0f, 1.0f },   //Color
            { 0.0f, 0.0f }                //UV-coords
        },
        {
            { 1.0f, -1.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 4.0f, 0.0f }
        },
        {
            { 1.0f, 1.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 4.0f, 4.0f }
        },
        {
            { -1.0f, 1.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 4.0f }
        }
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Alt: Update just the last vertex in the buffer

    //struct vertex new_subvertex;
    //glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) - sizeof(new_subvertex),
    //                    sizeof(new_subvertex), &new_subvertex);

    //--------------------------------------------------------------------------
    // Bind element data to buffer
    //--------------------------------------------------------------------------

    // Bind element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[VBO_ELEMENT]);

    GLuint elements[6] = { 0, 1, 3, 1, 2, 3 };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
                 GL_STATIC_DRAW);

    // Alt: Update just the last element index

    //GLuint new_subelement;
    //glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
    //                sizeof(elements) - sizeof(new_subelement),
    //                sizeof(new_subelement), &new_subelement);

    //--------------------------------------------------------------------------
    // Create vertex shader
    //--------------------------------------------------------------------------
    enum { SHADER_VERTEX, SHADER_FRAGMENT };

    //TODO: Read shader file into buffer
    GLchar *vshader_buf =
        "#version 330 core\n"

        "layout(location = 0) in vec4 vert_pos;\n"
        "layout(location = 1) in vec4 vert_col;\n"
        "layout(location = 2) in vec2 vert_uv;\n"

        "out vec4 frag_col;\n"
        "out vec2 frag_uv;\n"

        "uniform float u_time;\n"

        "uniform mat4 u_model;\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_proj;\n"

        "void main()\n"
        "{\n"
        "    gl_Position = u_proj * u_view * u_model * vec4(vert_pos);\n"
        //"    gl_Position = vec4(position) * model * view * proj;\n"
        "    frag_col = vert_col;\n"
        "    frag_uv = vert_uv;\n"
        "}\n\0";
    GLint vshader_len = strlen(vshader_buf);
    
    //printf("\nVertex Shader:\n%s\n", vshader_buf);

    /*************************************************************************
    | Shader types:
    | 
    | GL_VERTEX_SHADER      Vertex shader.
    | GL_GEOMETRY_SHADER    Geometry shader.
    | GL_FRAGMENT_SHADER    Fragment shader.
    |
    *************************************************************************/
    shaders[SHADER_VERTEX] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaders[SHADER_VERTEX], 1, (const GLchar**)&vshader_buf,
                   &vshader_len);

    //TODO: Uncomment this when loading shader from file
    //free(vshader_buf);

    GLint vshader_status;
    glCompileShader(shaders[SHADER_VERTEX]);
    glGetShaderiv(shaders[SHADER_VERTEX], GL_COMPILE_STATUS, &vshader_status);
    
    // Handle shader compile errors
    if (!vshader_status)
    {
        GLsizei vshaderlog_len;
        GLchar *vshaderlog;

        glGetShaderiv(shaders[SHADER_FRAGMENT], GL_INFO_LOG_LENGTH,
                      &vshaderlog_len);
        vshaderlog = malloc(vshaderlog_len);
        glGetShaderInfoLog(shaders[SHADER_FRAGMENT], vshaderlog_len, NULL,
                           vshaderlog);
        fprintf(stderr, "%s", vshaderlog);
        free(vshaderlog);
        return;
    }

    //--------------------------------------------------------------------------
    // Create fragment shader
    //--------------------------------------------------------------------------

    //TODO: Read shader file into buffer
    const char *fshader_buf =
        "#version 330 core\n"

        "in vec4 frag_col;\n"
        "in vec2 frag_uv;\n"

        "out vec4 col;\n"

        "uniform sampler2D u_tex;\n"

        "void main()\n"
        "{\n"
        "    col = texture(u_tex, frag_uv);\n"// + frag_col;\n"
        "}\n\0";
    GLint fshader_len = strlen(fshader_buf);

    shaders[SHADER_FRAGMENT] = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaders[SHADER_FRAGMENT], 1, (const GLchar**)&fshader_buf,
                   &fshader_len);

    //TODO: Uncomment this when loading shader from file
    //free(fshader_buf);

    GLint fshader_status;
    glCompileShader(shaders[SHADER_FRAGMENT]);
    glGetShaderiv(shaders[SHADER_FRAGMENT], GL_COMPILE_STATUS, &fshader_status);

    // Handle shader compile errors
    if (!fshader_status)
    {
        GLsizei fshaderlog_len;
        GLchar *fshaderlog;

        glGetShaderiv(shaders[SHADER_FRAGMENT], GL_INFO_LOG_LENGTH,
                      &fshaderlog_len);
        fshaderlog = malloc(fshaderlog_len);
        glGetShaderInfoLog(shaders[SHADER_FRAGMENT], fshaderlog_len, NULL,
                           fshaderlog);
        fprintf(stderr, "%s", fshaderlog);
        free(fshaderlog);
        return;
    }

    //--------------------------------------------------------------------------
    // Create shader program
    //--------------------------------------------------------------------------

    // Link shader program
    program_id = glCreateProgram();
    glAttachShader(program_id, shaders[SHADER_VERTEX]);
    glAttachShader(program_id, shaders[SHADER_FRAGMENT]);

    // Bind vertex shader output to fragment shader color number
    // (color < GL_MAX_DRAW_BUFFERS)
    //glBindFragDataLocation(program, 0, "v_color");

    glLinkProgram(program_id);

    // Clean up shaders
    glDetachShader(program_id, shaders[SHADER_VERTEX]);
    glDetachShader(program_id, shaders[SHADER_FRAGMENT]);

    glDeleteShader(shaders[SHADER_VERTEX]);
    glDeleteShader(shaders[SHADER_FRAGMENT]);

    GLint program_status;
    glGetProgramiv(program_id, GL_LINK_STATUS, &program_status);

    // Handle program link errors
    if (!program_status)
    {
        GLsizei programlog_len;
        GLchar *programlog;

        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH,
                      &programlog_len);
        programlog = malloc(programlog_len);
        glGetProgramInfoLog(program_id, programlog_len, NULL,
                            programlog);
        fprintf(stderr, "%s", programlog);
        free(programlog);

        //Clean up
        glDeleteProgram(program_id);
        return;
    }

    //--------------------------------------------------------------------------
    // Get vertex shader attribute locations and set pointers (size/type/stride)
    //--------------------------------------------------------------------------
    GLint attrib_pos = glGetAttribLocation(program_id, "vert_pos");
    GLint attrib_col = glGetAttribLocation(program_id, "vert_col");
    GLint attrib_uv = glGetAttribLocation(program_id, "vert_uv");

    if (attrib_pos >= 0)
    {
        glVertexAttribPointer(attrib_pos, 4, GL_FLOAT, GL_FALSE,
                              10*sizeof(GL_FLOAT),
                              (GLvoid *)(0));
        glEnableVertexAttribArray(attrib_pos);
    }
    else
    {
        fprintf(stderr, "No 'vert_pos' attribute in shader. "
                        "Possibly optimized out.\n");
    }

    if (attrib_col >= 0)
    {
        glVertexAttribPointer(attrib_col, 3, GL_FLOAT, GL_FALSE,
                              10 * sizeof(GL_FLOAT),
                              (GLvoid *)(4 * sizeof(GL_FLOAT)));
        glEnableVertexAttribArray(attrib_col);
    }
    else
    {
        fprintf(stderr, "No 'vert_col' attribute in shader. "
                        "Possibly optimized out.\n");
    }

    if (attrib_uv >= 0)
    {
        glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE,
                              10 * sizeof(GL_FLOAT),
                              (GLvoid *)(8 * sizeof(GL_FLOAT)));
        glEnableVertexAttribArray(attrib_uv);
    }
    else
    {
        fprintf(stderr, "No 'vert_uv' attribute in shader. "
                        "Possibly optimized out.\n");
    }

    // Alt: Specify VAO

    //glEnableVertexArrayAttrib(vao, attrib_pos);

    //--------------------------------------------------------------------------
    // Store uniform locations
    //--------------------------------------------------------------------------
    glUseProgram(program_id);

    uniform_time = glGetUniformLocation(program_id, "u_time");
    uniform_model = glGetUniformLocation(program_id, "u_model");
    uniform_view = glGetUniformLocation(program_id, "u_view");
    
    //TODO: Error check uniform existence

    //--------------------------------------------------------------------------
    // Calculate perspective projection matrix
    //--------------------------------------------------------------------------

    //Projection matrix
    uniform_proj = glGetUniformLocation(program_id, "u_proj");

    GLfloat screen_w = 1024.0f;
    GLfloat screen_h = 768.0f;
    GLfloat z_near = -1.0f;
    GLfloat z_far = -100.0f;
    GLfloat fovDeg = 50.0f;

    GLfloat aspect = screen_w / screen_h;
    GLfloat z_range = z_far - z_near;
    GLfloat fovRads = fovDeg * (float)M_PI / 180.0f;
    GLfloat fovCalc = 1.0f / (float)tan(fovRads / 2.0f);

    mat5 proj_matrix = mat5_create(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    //Calculate PERSPECTIVE projection
    proj_matrix[M00] = fovCalc / aspect;
    proj_matrix[M11] = fovCalc;
    proj_matrix[M22] = (z_far + z_near) / z_range;
    proj_matrix[M23] = -1.0f;
    proj_matrix[M32] = 2.0f * (z_far * z_near) / z_range;

    glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, proj_matrix);

    glUseProgram(0);

    //--------------------------------------------------------------------------
    // Generate textures
    //--------------------------------------------------------------------------

    glUseProgram(program_id);
    uniform_tex = glGetUniformLocation(program_id, "u_tex");

    glGenTextures(1, &tex_r);
    glGenTextures(1, &tex_g);

    /*************************************************************************
    | Common texture binding targets:
    |   
    | GL_TEXTURE_1D         0.0f -> 1.0f (x)
    | GL_TEXTURE_2D         0.0f -> 1.0f (x, y)
    | GL_TEXTURE_3D         0.0f -> 1.0f (x, y, z)
    | GL_TEXTURE_RECTANGLE  0.0f -> width (x), 0.0f -> height (y)
    | GL_TEXTURE_CUBE_MAP   Six TEXTURE_2D where all width/height are equal.
    |
    **************************************************************************
    | Other targets:
    |   
    | GL_TEXTURE_2D_MULTISAMPLE        Multiple samples (colors) per pixel.
    | GL_TEXTURE_1D_ARRAY              Alt: Multiple texture arrays.
    | GL_TEXTURE_2D_ARRAY               |
    | GL_TEXTURE_2D_MULTISAMPLE_ARRAY   |
    | GL_TEXTURE_CUBE_MAP_ARRAY         v
    |
    *************************************************************************/
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_r);

    //--------------------------------------------------------------------------
    // Configure texture wrapping (coordinates outside range 0.0f - 1.0f)
    //--------------------------------------------------------------------------

    /*************************************************************************
    | Wrapping modes:
    | 
    | GL_REPEAT            Repeat texture w/ same orientation.
    | GL_MIRRORED_REPEAT   Repeat texture mirrored over adjacted edges.
    | GL_CLAMP_TO_EDGE     Clamp to edge of geometry (stretch edges).
    | GL_CLAMP_TO_BORDER   Clamp to edge of texture (no stretching).
    |
    *************************************************************************/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

    // For GL_CLAMP_TO_BORDER
    //float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

    //--------------------------------------------------------------------------
    // Configure texture filtering
    //--------------------------------------------------------------------------

    /*************************************************************************
    | Filering modes:
    |
    | GL_NEAREST                Nearest pixel to coordinate.
    | GL_LINEAR                 Linear interpolation of nearest neighbors
    |                           (4 pixels in 2D).
    | GL_NEAREST_MIPMAP_NEAREST Nearest mipmap, nearest pixel.
    | GL_NEAREST_MIPMAP_LINEAR  Nearest mipmap, interpolate neighbor pixels.
    | GL_LINEAR_MIPMAP_NEAREST  Average 2 best mipmaps, nearest pixel.
    | GL_LINEAR_MIPMAP_LINEAR   Average 2 best mipmaps, interpolate neighbor
    |                           pixels.
    |
    *************************************************************************/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //--------------------------------------------------------------------------
    // Load texture data
    //--------------------------------------------------------------------------

    GLsizei tex_w, tex_h, bpp;
    unsigned char* pixels = stbi_load("basic.tga", &tex_w, &tex_h, &bpp, 4);
    if (!pixels)
    {
        fprintf(stderr, "Failed to load texture: %s", "basic.tga");
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);

    stbi_image_free(pixels);

    //GLsizei tex_w, tex_h;
    //unsigned char* pixels = read_tga("basic.tga", &tex_w, &tex_h);
    //
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_w, tex_h, 0, GL_BGR,
    //             GL_UNSIGNED_BYTE, pixels);
    //
    //free(pixels);

    //--------------------------------------------------------------------------

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_g);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLsizei tex_w_g, tex_h_g, bpp_g;
    unsigned char* pixels_g = stbi_load("basic.tga", &tex_w_g, &tex_h_g,
                                        &bpp_g, 4);
    if (!pixels_g)
    {
        fprintf(stderr, "Failed to load texture: %s", "basic.tga");
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w_g, tex_h_g, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels_g);
    stbi_image_free(pixels_g);

    glUseProgram(0);

    //--------------------------------------------------------------------------
    // Generate mipmaps for the current texture
    //--------------------------------------------------------------------------
    //glGenerateMipmap(GL_TEXTURE_2D);

    glBindVertexArray(0);
}

void update_glref(GLfloat dt)
{
    //--------------------------------------------------------------------------
    // Update uniforms
    //--------------------------------------------------------------------------

    /*************************************************************************
    | Row Major
    | model = trans * rot * scale
    | view = trans * rot * scale
    | gl_Position = proj * view * model * vec
    |
    **************************************************************************
    | Column Major
    | model = scale * rot * trans
    | view = scale * rot * trans
    | gl_Position = vec * model * view * proj
    |
    *************************************************************************/

    glUseProgram(program_id);

    //Delta time
    uniform_time = glGetUniformLocation(program_id, "u_time");
    glUniform1f(uniform_time, dt);

    uniform_model = glGetUniformLocation(program_id, "u_model");
    uniform_view = glGetUniformLocation(program_id, "u_view");

    mat5 view_matrix = mat5_create_ident();
    mat5_scale(view_matrix, view_scale);
    mat5_rotx(view_matrix, view_rot.x);
    mat5_roty(view_matrix, view_rot.y);
    mat5_rotz(view_matrix, view_rot.z);
    mat5_translate(view_matrix, view_trans);
    //mat5_mul_into_a(&view_matrix, mat5_create_scale(view_scale));
    //mat5_mul_into_a(&view_matrix, mat5_create_rotx(view_rot.x));
    //mat5_mul_into_a(&view_matrix, mat5_create_roty(view_rot.y));
    //mat5_mul_into_a(&view_matrix, mat5_create_rotz(view_rot.z));
    //mat5_mul_into_a(&view_matrix, mat5_create_translate(view_trans));

    glUniformMatrix4fv(uniform_view, 1, GL_FALSE, view_matrix);

    glUseProgram(0);
}

void render_glref()
{
    //--------------------------------------------------------------------------
    // Draw
    //--------------------------------------------------------------------------
    
    glUseProgram(program_id);
    glBindVertexArray(vao);
    
    //TODO: Don't have multiple textures / model matrices for single mesh
    //      Let the mesh render itself, set uniforms in mesh_init()

    mat5 model_matrix;

    model_matrix = mat5_create_ident();
    mat5_translate(model_matrix, (struct vec4) { 0.0f, 0.0f, -3.0f });
    mat5_scale(model_matrix, (struct vec4) { 1.0f, 1.0f, 1.0f });
    glUniformMatrix4fv(uniform_model, 1, GL_FALSE, model_matrix);

    glUniform1i(uniform_tex, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    model_matrix = mat5_create_ident();
    mat5_translate(model_matrix, (struct vec4) { 0.0f, 1.0f, -4.0f });
    mat5_roty(model_matrix, 30.0f);
    mat5_scale(model_matrix, (struct vec4) { 1.0f, 2.0f, 1.0f });
    glUniformMatrix4fv(uniform_model, 1, GL_FALSE, model_matrix);

    glUniform1i(uniform_tex, 1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void free_glref()
{
    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    glDeleteTextures(1, &tex_r);
    glDeleteTextures(1, &tex_g);
    glDeleteBuffers(2, vbos);
    glDeleteProgram(program_id);
    glDeleteVertexArrays(1, &vao);
}