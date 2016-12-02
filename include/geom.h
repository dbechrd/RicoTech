#ifndef GEOM_H
#define GEOM_H

#include "const.h"
#include <GL/gl3w.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
// RGBA color
//------------------------------------------------------------------------------
struct col4 {
    GLfloat r, g, b, a;
};

//Predefined colors
extern const struct col4 COLOR_BLACK;
extern const struct col4 COLOR_RED;
extern const struct col4 COLOR_GREEN;
extern const struct col4 COLOR_BLUE;
extern const struct col4 COLOR_YELLOW;
extern const struct col4 COLOR_CYAN;
extern const struct col4 COLOR_MAGENTA;
extern const struct col4 COLOR_WHITE;
extern const struct col4 COLOR_GRAY;

extern const struct col4 COLOR_BLACK_HIGHLIGHT;
extern const struct col4 COLOR_RED_HIGHLIGHT;
extern const struct col4 COLOR_GREEN_HIGHLIGHT;
extern const struct col4 COLOR_BLUE_HIGHLIGHT;
extern const struct col4 COLOR_YELLOW_HIGHLIGHT;
extern const struct col4 COLOR_CYAN_HIGHLIGHT;
extern const struct col4 COLOR_MAGENTA_HIGHLIGHT;
extern const struct col4 COLOR_WHITE_HIGHLIGHT;
extern const struct col4 COLOR_GRAY_HIGHLIGHT;

extern const struct col4 COLOR_DARK_RED;
extern const struct col4 COLOR_DARK_GREEN;
extern const struct col4 COLOR_DARK_BLUE;
extern const struct col4 COLOR_DARK_YELLOW;
extern const struct col4 COLOR_DARK_CYAN;
extern const struct col4 COLOR_DARK_MAGENTA;
extern const struct col4 COLOR_DARK_GRAY;

//------------------------------------------------------------------------------
// 2D Texture coordinates
//------------------------------------------------------------------------------
struct tex2 {
    GLfloat u, v;
};

//------------------------------------------------------------------------------
// 4D Vector
//------------------------------------------------------------------------------
struct vec4 {
    GLfloat x, y, z, w;
};

extern const struct vec4 VEC4_ZERO;
extern const struct vec4 VEC4_UNIT;

static inline struct vec4 vec_add(const struct vec4 a, const struct vec4 b)
{
    return (struct vec4) {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
        1.0f
    };
}
static inline struct vec4 vec_sub(const struct vec4 a, const struct vec4 b)
{
    return (struct vec4) {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
        1.0f
    };
}
static inline struct vec4 vec_scale(const struct vec4 v, float scale)
{
    return (struct vec4) {
        v.x * scale,
        v.y * scale,
        v.z * scale,
        1.0f
    };
}
static inline struct vec4 vec_cross(const struct vec4 a, const struct vec4 b)
{
    return (struct vec4) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
        1.0f
    };
}
static inline GLfloat vec_length(const struct vec4 v)
{
    return sqrtf(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
    );
}
static inline struct vec4 vec_negate(const struct vec4 v)
{
    return (struct vec4) { -v.x, -v.y, -v.z, v.w };
}
static inline struct vec4 vec_normalize(const struct vec4 v)
{
    GLfloat len = 1.0f / vec_length(v);

    return (struct vec4) {
        v.x * len,
        v.y * len,
        v.z * len,
        0.0f
    };
}
static inline bool vec_equals(const struct vec4 a, const struct vec4 b)
{
    return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
}

//------------------------------------------------------------------------------
// 4D Matrix
//------------------------------------------------------------------------------
struct mat4 {
    union {
        struct vec4 v[4]; //Vectors
        GLfloat a[16];    //Array
        GLfloat m[4][4];  //Matrix
        /*struct {
            GLfloat m00, m01, m02, m03;
            GLfloat m10, m11, m12, m13;
            GLfloat m20, m21, m22, m23;
            GLfloat m30, m31, m32, m33;
        };*/
    };
};

extern const struct mat4 MAT4_IDENT;

//Store as column-major, one-dimensional array of floats
static inline struct mat4 make_mat4(
    GLfloat m00, GLfloat m01, GLfloat m02, GLfloat m03,
    GLfloat m10, GLfloat m11, GLfloat m12, GLfloat m13,
    GLfloat m20, GLfloat m21, GLfloat m22, GLfloat m23,
    GLfloat m30, GLfloat m31, GLfloat m32, GLfloat m33)
{
    struct mat4 mat;
    mat.m[0][0] = m00;
    mat.m[1][0] = m10;
    mat.m[2][0] = m20;
    mat.m[3][0] = m30;
    mat.m[0][1] = m01;
    mat.m[1][1] = m11;
    mat.m[2][1] = m21;
    mat.m[3][1] = m31;
    mat.m[0][2] = m02;
    mat.m[1][2] = m12;
    mat.m[2][2] = m22;
    mat.m[3][2] = m32;
    mat.m[0][3] = m03;
    mat.m[1][3] = m13;
    mat.m[2][3] = m23;
    mat.m[3][3] = m33;
    return mat;
}

// static inline struct mat4 make_mat4_ident()
// {
//     return MAT4_IDENT;
// }
static inline void mat4_ident(struct mat4 *m)
{
    *m = MAT4_IDENT;
    // m->m[0][0] = 1;
    // m->m[1][0] = 0;
    // m->m[2][0] = 0;
    // m->m[3][0] = 0;
    // m->m[0][1] = 0;
    // m->m[1][1] = 1;
    // m->m[2][1] = 0;
    // m->m[3][1] = 0;
    // m->m[0][2] = 0;
    // m->m[1][2] = 0;
    // m->m[2][2] = 1;
    // m->m[3][2] = 0;
    // m->m[0][3] = 0;
    // m->m[1][3] = 0;
    // m->m[2][3] = 0;
    // m->m[3][3] = 1;
}

static inline struct mat4 make_mat4_translate(struct vec4 v)
{
    return make_mat4(
        1, 0, 0, v.x,
        0, 1, 0, v.y,
        0, 0, 1, v.z,
        0, 0, 0, 1
    );
}
static inline void mat4_translate(struct mat4 *m, struct vec4 v)
{
    m->m[0][3] += m->m[0][0] * v.x + m->m[0][1] * v.y + m->m[0][2] * v.z;
    m->m[1][3] += m->m[1][0] * v.x + m->m[1][1] * v.y + m->m[1][2] * v.z;
    m->m[2][3] += m->m[2][0] * v.x + m->m[2][1] * v.y + m->m[2][2] * v.z;
    m->m[3][3] += m->m[3][0] * v.x + m->m[3][1] * v.y + m->m[3][2] * v.z;
}

//static inline void mat4_translate_other(mat4 m, struct vec4 v)
//{
//    m[M30] += m[M00]*v.x + m[M10]*v.y + m[M20]*v.z;
//    m[M31] += m[M01]*v.x + m[M11]*v.y + m[M21]*v.z;
//    m[M32] += m[M02]*v.x + m[M12]*v.y + m[M22]*v.z;
//    m[M33] += m[M03]*v.x + m[M13]*v.y + m[M23]*v.z;
//}

static inline struct mat4 make_mat4_scale(struct vec4 s)
{
    return make_mat4(
        s.x, 0, 0, 0,
        0, s.y, 0, 0,
        0, 0, s.z, 0,
        0, 0, 0, 1
    );
}
static inline void mat4_scale(struct mat4 *m, struct vec4 s)
{
    m->m[0][0] *= s.x;
    m->m[1][0] *= s.x;
    m->m[2][0] *= s.x;
    m->m[3][0] *= s.x;
    m->m[0][1] *= s.y;
    m->m[1][1] *= s.y;
    m->m[2][1] *= s.y;
    m->m[3][1] *= s.y;
    m->m[0][2] *= s.z;
    m->m[1][2] *= s.z;
    m->m[2][2] *= s.z;
    m->m[3][2] *= s.z;
}

static inline struct mat4 make_mat4_rotx(float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);
    return make_mat4(
        1, 0, 0, 0,
        0, c, -s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    );
}
static inline void mat4_rotx(struct mat4 *m, float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);

    GLfloat tmp;

    tmp = m->m[0][1];
    m->m[0][1] = m->m[0][1] * c + m->m[0][2] * s;
    m->m[0][2] = m->m[0][2] * c + tmp * -s;

    tmp = m->m[1][1];
    m->m[1][1] = m->m[1][1] * c + m->m[1][2] * s;
    m->m[1][2] = m->m[1][2] * c + tmp * -s;

    tmp = m->m[2][1];
    m->m[2][1] = m->m[2][1] * c + m->m[2][2] * s;
    m->m[2][2] = m->m[2][2] * c + tmp * -s;

    tmp = m->m[3][1];
    m->m[3][1] = m->m[3][1] * c + m->m[3][2] * s;
    m->m[3][2] = m->m[3][2] * c + tmp * -s;
}

static inline struct mat4 make_mat4_roty(float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);
    return make_mat4(
        c, 0, s, 0,
        0, 1, 0, 0,
        -s, 0, c, 0,
        0, 0, 0, 1
    );
}
static inline void mat4_roty(struct mat4 *m, float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);

    GLfloat tmp;

    tmp = m->m[0][0];
    m->m[0][0] = m->m[0][0] * c + m->m[0][2] * -s;
    m->m[0][2] = tmp*s + m->m[0][2] * c;

    tmp = m->m[1][0];
    m->m[1][0] = m->m[1][0] * c + m->m[1][2] * -s;
    m->m[1][2] = tmp*s + m->m[1][2] * c;

    tmp = m->m[2][0];
    m->m[2][0] = m->m[2][0] * c + m->m[2][2] * -s;
    m->m[2][2] = tmp*s + m->m[2][2] * c;

    tmp = m->m[3][0];
    m->m[3][0] = m->m[3][0] * c + m->m[3][2] * -s;
    m->m[3][2] = tmp*s + m->m[3][2] * c;
}

static inline struct mat4 make_mat4_rotz(float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);
    return make_mat4(
        c, -s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}
static inline void mat4_rotz(struct mat4 *m, float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);

    GLfloat tmp;

    tmp = m->m[0][0];
    m->m[0][0] = m->m[0][0] * c + m->m[0][1] * s;
    m->m[0][1] = m->m[0][1] * c + tmp * -s;

    tmp = m->m[1][0];
    m->m[1][0] = m->m[1][0] * c + m->m[1][1] * s;
    m->m[1][1] = m->m[1][1] * c + tmp * -s;

    tmp = m->m[2][0];
    m->m[2][0] = m->m[2][0] * c + m->m[2][1] * s;
    m->m[2][1] = m->m[2][1] * c + tmp * -s;

    tmp = m->m[3][0];
    m->m[3][0] = m->m[3][0] * c + m->m[3][1] * s;
    m->m[3][1] = m->m[3][1] * c + tmp * -s;
}

static inline struct mat4 make_mat4_transpose(const struct mat4 *m)
{
    return make_mat4(
        m->m[0][0], m->m[1][0], m->m[2][0], m->m[3][0],
        m->m[0][1], m->m[1][1], m->m[2][1], m->m[3][1],
        m->m[0][2], m->m[1][2], m->m[2][2], m->m[3][2],
        m->m[0][3], m->m[1][3], m->m[2][3], m->m[3][3]
    );
}
static inline void mat4_transpose(struct mat4 *m)
{
    GLfloat tmp;

    tmp = m->m[0][1];
    m->m[0][1] = m->m[1][0];
    m->m[1][0] = tmp;

    tmp = m->m[0][2];
    m->m[0][2] = m->m[2][0];
    m->m[2][0] = tmp;

    tmp = m->m[0][3];
    m->m[0][3] = m->m[3][0];
    m->m[3][0] = tmp;

    tmp = m->m[1][2];
    m->m[1][2] = m->m[2][1];
    m->m[2][1] = tmp;

    tmp = m->m[1][3];
    m->m[1][3] = m->m[3][1];
    m->m[3][1] = tmp;

    tmp = m->m[2][3];
    m->m[2][3] = m->m[3][2];
    m->m[3][2] = tmp;
}

//Calculate PERSPECTIVE projection
static inline struct mat4 make_mat4_perspective(float width, float height,
                                                float z_near, float z_far,
                                                float fov_deg)
{
    float aspect = width / height;
    float z_range = (z_far - z_near);
    float fov_rads = fov_deg * (float)M_PI / 180.0f;
    float fov_calc = 1.0f / (float)tan(fov_rads / 2.0f);

    // Flip Z-axis so that +Z is toward the player
    z_range *= -1.0f;

    struct mat4 mat = MAT4_IDENT;
    mat.m[0][0] = fov_calc / aspect;
    mat.m[1][1] = fov_calc;
    mat.m[2][2] = (z_far + z_near) / z_range;
    mat.m[2][3] = -1.0f;
    mat.m[3][2] = 2.0f * (z_far * z_near) / z_range;
    return mat;
}

static inline void mat4_mul(const struct mat4 *a, const struct mat4 *b,
                            struct mat4 *result)
{
    for (int aj = 0; aj < 4; ++aj)
    {
        for (int bi = 0; bi < 4; ++bi)
        {
            result->m[aj][bi] = 0;
            for (int n = 0; n < 4; ++n)
            {
                result->m[aj][bi] += a->m[aj][n] * b->m[n][bi];
            }
        }
    }
}

static inline int mat4_equals(const struct mat4 *a, const struct mat4 *b)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (a->m[i][j] != b->m[i][j])
                return 0;
        }
    }
    return 1;
}

//Debug: Print matrix in row-major form
static inline void mat4_print(const struct mat4 *m)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            printf("%f ", m->m[j][i]);
        }
        printf("\n");
    }
    printf("\n");
}

#endif