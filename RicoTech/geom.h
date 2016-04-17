#ifndef GEOM_H
#define GEOM_H

#include <GL/gl3w.h>

#ifndef PI
#define PI 3.141592653589793
#endif

#ifndef PI2
#define PI2 6.28318530717959
#endif

typedef struct Vec4 {
    GLfloat x, y, z, w;
} Vec4;

Vec4 *Vec4_create(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void Vec4_destroy(Vec4 *vec);

typedef struct Vec3 {
    GLfloat x, y, z;
} Vec3;

Vec3 *Vec3_create(GLfloat x, GLfloat y, GLfloat z);
void Vec3_destroy(Vec3 *vec);

typedef struct RegularPoly {
    Vec3 pos;
    GLfloat radius;

    Vec4 *vertices;
    unsigned int count;
    GLuint vao;
} RegularPoly;

RegularPoly *RegularPoly_create(const Vec3 center, const GLfloat radius, const unsigned int vertex_count);
void RegularPoly_destroy(RegularPoly *poly);

void RegularPoly_move(RegularPoly *poly, const GLfloat x, const GLfloat y, const GLfloat z);
void RegularPoly_render(RegularPoly *poly);

#endif