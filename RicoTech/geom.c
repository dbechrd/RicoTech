#include "geom.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

Vec4 *Vec4_create(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    Vec4 *vec = malloc(sizeof(Vec4));
    vec->x = x;
    vec->y = y;
    vec->z = z;
    vec->w = w;

    return vec;
}

void Vec4_destroy(Vec4 *vec)
{
    free(vec);
    vec = NULL;
}

Vec3 *Vec3_create(GLfloat x, GLfloat y, GLfloat z)
{
    Vec3 *vec = malloc(sizeof(Vec3));
    vec->x = x;
    vec->y = y;
    vec->z = z;

    return vec;
}

void Vec3_destroy(Vec3 *vec)
{
    free(vec);
    vec = NULL;
}

static char regularpoly_init = 0;
static GLuint regularpoly_program;
static GLint regularpoly_prog_attrib_position;

static void RegularPoly_rebuildVAO(RegularPoly *poly);

char RegularPoly_init()
{
    GLuint poly_vshader = make_shader(
        GL_VERTEX_SHADER,
        "poly.v.glsl"
    );
    if (!poly_vshader)
    {
        fprintf(stderr, "RegularPoly: Failed to make vertex shader.\n");
        return 0;
    }

    GLuint poly_fshader = make_shader(
        GL_FRAGMENT_SHADER,
        "poly.f.glsl"
    );
    if (!poly_fshader)
    {
        fprintf(stderr, "RegularPoly: Failed to make fragment shader.\n");
        return 0;
    }

    regularpoly_program = make_program(
        poly_vshader,
        poly_fshader
    );
    if (!regularpoly_program)
    {
        fprintf(stderr, "RegularPoly: Failed to make program.\n");
        return 0;
    }

    regularpoly_prog_attrib_position = glGetAttribLocation(regularpoly_program, "position");
    if (regularpoly_prog_attrib_position == -1)
    {
        fprintf(stderr, "RegularPoly: Failed to get atrribute location for position.\n");
        return 0;
    }

    return 1;
}

RegularPoly *RegularPoly_create(const Vec3 center, const GLfloat radius, const unsigned int vertex_count)
{
    if (!regularpoly_init && !RegularPoly_init())
    {
        fprintf(stderr, "RegularPoly: Init failed.\n");
        return NULL;
    }

    RegularPoly *poly = malloc(sizeof(RegularPoly));

    poly->pos = center;
    poly->radius = radius;

    poly->vertices = malloc(sizeof(Vec4) * vertex_count);
    poly->count = vertex_count;

    poly->vao = 0;
    RegularPoly_rebuildVAO(poly);

    return poly;
}

void RegularPoly_destroy(RegularPoly *poly)
{
    free(poly->vertices);
    poly->vertices = NULL;

    free(poly);
    poly = NULL;
}

void RegularPoly_move(RegularPoly *poly, const GLfloat x, const GLfloat y, const GLfloat z)
{
    poly->pos.x = x;
    poly->pos.y = y;
    poly->pos.z = z;

    RegularPoly_rebuildVAO(poly);
}

static void RegularPoly_rebuildVAO(RegularPoly *poly)
{
    if (poly->vao)
    {
        glDeleteVertexArrays(1, &poly->vao);
    }

    double delta_angle = PI2 / (double)poly->count;
    double angle = 0;

    for (unsigned int i = 0; i < poly->count; i++)
    {
        poly->vertices[i].x = (GLfloat)cos(angle) * poly->radius + poly->pos.x;
        poly->vertices[i].y = (GLfloat)sin(angle) * poly->radius + poly->pos.y;
        poly->vertices[i].z = poly->pos.z;
        poly->vertices[i].w = 1.0f;
        angle += delta_angle;
    }

    glGenVertexArrays(1, &poly->vao);
    glBindVertexArray(poly->vao);

    GLuint poly_vbo;

    glGenBuffers(1, &poly_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, poly_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(poly->vertices[0])*poly->count, poly->vertices, GL_STATIC_DRAW);

    //(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    glVertexAttribPointer(regularpoly_prog_attrib_position, 4, GL_FLOAT, GL_FALSE, 0, 0);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, poly_elements);
    glEnableVertexAttribArray(regularpoly_prog_attrib_position);

    glBindVertexArray(0);
    glDeleteBuffers(1, &poly_vbo);
}

void RegularPoly_render(RegularPoly *poly)
{
    glUseProgram(regularpoly_program);
    glBindVertexArray(poly->vao);

    //(GLenum mode, GLint first, GLsizei count)
    glDrawArrays(
        GL_TRIANGLE_FAN,
        0,
        poly->count
    );

    glBindVertexArray(0);
}