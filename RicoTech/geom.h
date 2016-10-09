#ifndef GEOM_H
#define GEOM_H

#include "program.h"
#include <GL/gl3w.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

#ifndef BBOX_EPSILON
#define BBOX_EPSILON 0.1f
#endif

struct col4 {
    GLfloat r, g, b, a; //TODO: Is a, r, g, b better?
};

static struct col4 COLOR_BLACK   = { 0.0f, 0.0f, 0.0f, 1.0f };
static struct col4 COLOR_RED     = { 1.0f, 0.0f, 0.0f, 1.0f };
static struct col4 COLOR_GREEN   = { 0.0f, 1.0f, 0.0f, 1.0f };
static struct col4 COLOR_BLUE    = { 0.0f, 0.0f, 1.0f, 1.0f };
static struct col4 COLOR_YELLOW  = { 1.0f, 1.0f, 0.0f, 1.0f };
static struct col4 COLOR_CYAN    = { 0.0f, 1.0f, 1.0f, 1.0f };
static struct col4 COLOR_MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };
static struct col4 COLOR_WHITE   = { 1.0f, 1.0f, 1.0f, 1.0f };

struct tex2 {
    GLfloat u, v;
};

////////////////////////////////////////////////////////////////////////////////

struct vec4 {
    GLfloat x, y, z, w;
};

////////////////////////////////////////////////////////////////////////////////

static inline struct vec4 vec_cross(struct vec4 a, struct vec4 b)
{
    struct vec4 r;
    r.x = a.y*b.z - a.z*b.y;
    r.y = a.z*b.x - a.x*b.z;
    r.z = a.x*b.y - a.y*b.x;
    r.w = 1.0f;

    return r;
}
static inline GLfloat vec_length(struct vec4 v)
{
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}
static inline struct vec4 vec_normalize(struct vec4 v)
{
    GLfloat len = 1.0f / vec_length(v);
    v.x *= len;
    v.y *= len;
    v.z *= len;
    v.w = 0.0f;

    return v;
}

////////////////////////////////////////////////////////////////////////////////

struct vertex {
    struct vec4 pos;
    struct col4 col;
    struct tex2 tex;
};

////////////////////////////////////////////////////////////////////////////////

#define M00 0
#define M10 1
#define M20 2
#define M30 3
#define M01 4
#define M11 5
#define M21 6
#define M31 7
#define M02 8
#define M12 9
#define M22 10
#define M32 11
#define M03 12
#define M13 13
#define M23 14
#define M33 15

typedef GLfloat *mat4;

//This is faster if the matrix is going to be immediately populated
static inline mat4 make_mat4_empty()
{
    return (mat4)malloc(16 * sizeof(mat4));
}

static inline mat4 make_mat4_zero()
{
    return (mat4)calloc(16, sizeof(mat4));
}

//Store as column-major, one-dimensional array of floats
static inline mat4 make_mat4(
    GLfloat m00, GLfloat m01, GLfloat m02, GLfloat m03,
    GLfloat m10, GLfloat m11, GLfloat m12, GLfloat m13,
    GLfloat m20, GLfloat m21, GLfloat m22, GLfloat m23,
    GLfloat m30, GLfloat m31, GLfloat m32, GLfloat m33)
{
    mat4 mat = make_mat4_empty();
    mat[M00] = m00; mat[M01] = m01; mat[M02] = m02; mat[M03] = m03;
    mat[M10] = m10; mat[M11] = m11; mat[M12] = m12; mat[M13] = m13;
    mat[M20] = m20; mat[M21] = m21; mat[M22] = m22; mat[M23] = m23;
    mat[M30] = m30; mat[M31] = m31; mat[M32] = m32; mat[M33] = m33;
    return mat;

    //mat4 mat = (mat4)malloc(16 * sizeof(mat4));
    //mat[0] = m00; mat[4] = m01;  mat[8] = m02; mat[12] = m03;
    //mat[1] = m10; mat[5] = m11;  mat[9] = m12; mat[13] = m13;
    //mat[2] = m20; mat[6] = m21; mat[10] = m22; mat[14] = m23;
    //mat[3] = m30; mat[7] = m31; mat[11] = m32; mat[15] = m33;
    //return mat;
}

static inline mat4 make_mat4_ident()
{
    return make_mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}
static inline void mat4_ident(mat4 m)
{
    m[M00] = 1;
    m[M10] = 0;
    m[M20] = 0;
    m[M30] = 0;
    m[M01] = 0;
    m[M11] = 1;
    m[M21] = 0;
    m[M31] = 0;
    m[M02] = 0;
    m[M12] = 0;
    m[M22] = 1;
    m[M32] = 0;
    m[M03] = 0;
    m[M13] = 0;
    m[M23] = 0;
    m[M33] = 1;
}

static inline mat4 make_mat4_translate(struct vec4 v)
{
    return make_mat4(
        1, 0, 0, v.x,
        0, 1, 0, v.y,
        0, 0, 1, v.z,
        0, 0, 0, 1
    );
}
static inline void mat4_translate(mat4 m, struct vec4 v)
{
    m[M03] += m[M00]*v.x + m[M01]*v.y + m[M02]*v.z;
    m[M13] += m[M10]*v.x + m[M11]*v.y + m[M12]*v.z;
    m[M23] += m[M20]*v.x + m[M21]*v.y + m[M22]*v.z;
    m[M33] += m[M30]*v.x + m[M31]*v.y + m[M32]*v.z;
}

//static inline void mat4_translate_other(mat4 m, struct vec4 v)
//{
//    m[M30] += m[M00]*v.x + m[M10]*v.y + m[M20]*v.z;
//    m[M31] += m[M01]*v.x + m[M11]*v.y + m[M21]*v.z;
//    m[M32] += m[M02]*v.x + m[M12]*v.y + m[M22]*v.z;
//    m[M33] += m[M03]*v.x + m[M13]*v.y + m[M23]*v.z;
//}

static inline mat4 make_mat4_scale(struct vec4 s)
{
    return make_mat4(
        s.x,   0,   0, 0,
          0, s.y,   0, 0,
          0,   0, s.z, 0,
          0,   0,   0, 1
    );
}
static inline void mat4_scale(mat4 m, struct vec4 s)
{
    m[M00] *= s.x;
    m[M10] *= s.x;
    m[M20] *= s.x;
    m[M30] *= s.x;
    m[M01] *= s.y;
    m[M11] *= s.y;
    m[M21] *= s.y;
    m[M31] *= s.y;
    m[M02] *= s.z;
    m[M12] *= s.z;
    m[M22] *= s.z;
    m[M32] *= s.z;
}

static inline mat4 make_mat4_rotx(float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);
    return make_mat4(
        1, 0, 0, 0,
        0, c,-s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    );
}
static inline void mat4_rotx(mat4 m, float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);

    GLfloat tmp;

    tmp = m[M01];
    m[M01] = m[M01]* c + m[M02]*s;
    m[M02] =    tmp*-s + m[M02]*c;

    tmp = m[M11];
    m[M11] = m[M11]* c + m[M12]*s;
    m[M12] =    tmp*-s + m[M12]*c;

    tmp = m[M21];
    m[M21] = m[M21]* c + m[M22]*s;
    m[M22] =    tmp*-s + m[M22]*c;

    tmp = m[M31];
    m[M31] = m[M31]* c + m[M32]*s;
    m[M32] =    tmp*-s + m[M32]*c;
}

static inline mat4 make_mat4_roty(float deg)
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
static inline void mat4_roty(mat4 m, float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);

    GLfloat tmp;

    tmp = m[M00];
    m[M00] = m[M00]*c + m[M02]*-s;
    m[M02] =    tmp*s + m[M02]* c;

    tmp = m[M10];
    m[M10] = m[M10]*c + m[M12]*-s;
    m[M12] =    tmp*s + m[M12]* c;

    tmp = m[M20];
    m[M20] = m[M20]*c + m[M22]*-s;
    m[M22] =    tmp*s + m[M22]* c;

    tmp = m[M30];
    m[M30] = m[M30]*c + m[M32]*-s;
    m[M32] =    tmp*s + m[M32]* c;
}

static inline mat4 make_mat4_rotz(float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);
    return make_mat4(
        c,-s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}
static inline void mat4_rotz(mat4 m, float deg)
{
    double r = deg * M_PI / 180;
    float s = (float)sin(r);
    float c = (float)cos(r);

    GLfloat tmp;

    tmp = m[M00];
    m[M00] = m[M00]* c + m[M01]*s;
    m[M01] =    tmp*-s + m[M01]*c;

    tmp = m[M10];
    m[M10] = m[M10]* c + m[M11]*s;
    m[M11] =    tmp*-s + m[M11]*c;

    tmp = m[M20];
    m[M20] = m[M20]* c + m[M21]*s;
    m[M21] =    tmp*-s + m[M21]*c;

    tmp = m[M30];
    m[M30] = m[M30]* c + m[M31]*s;
    m[M31] =    tmp*-s + m[M31]*c;
}

static inline mat4 make_mat4_transpose(const mat4 m)
{
    return make_mat4(
        m[M00], m[M10], m[M20], m[M30],
        m[M01], m[M11], m[M21], m[M31],
        m[M02], m[M12], m[M22], m[M32],
        m[M03], m[M13], m[M23], m[M33]
    );
}
static inline void mat4_transpose(mat4 m)
{
    GLfloat tmp;

    tmp = m[M01];
    m[M01] = m[M10];
    m[M10] = tmp;
    
    tmp = m[M02];
    m[M02] = m[M20];
    m[M20] = tmp;
    
    tmp = m[M03];
    m[M03] = m[M30];
    m[M30] = tmp;

    tmp = m[M12];
    m[M12] = m[M21];
    m[M21] = tmp;
    
    tmp = m[M13];
    m[M13] = m[M31];
    m[M31] = tmp;

    tmp = m[M23];
    m[M23] = m[M32];
    m[M32] = tmp;
}

//Calculate PERSPECTIVE projection
static inline mat4 make_mat4_perspective(float width, float height,
                                           float z_near, float z_far,
                                           float fov_deg)
{
    float aspect = width / height;
    float z_range = z_far - z_near;
    float fov_rads = fov_deg * (float)M_PI / 180.0f;
    float fov_calc = 1.0f / (float)tan(fov_rads / 2.0f);

    mat4 proj_matrix = make_mat4_ident();

    proj_matrix[M00] = fov_calc / aspect;
    proj_matrix[M11] = fov_calc;
    proj_matrix[M22] = (z_far + z_near) / z_range;
    proj_matrix[M23] = -1.0f;
    proj_matrix[M32] = 2.0f * (z_far * z_near) / z_range;

    return proj_matrix;
}

//TODO: const GLfloat *foo[4] or something?? Second dimension doesn't decay.

//This is about twice as fast as the loop version (~15ns vs. 30ns)
static inline void mat4_mul(const mat4 a, const mat4 b, mat4 result)
{
    result[M00] = a[M00]*b[M00] + a[M01]*b[M10] + a[M02]*b[M20] + a[M03]*b[M30];
    result[M10] = a[M10]*b[M00] + a[M11]*b[M10] + a[M12]*b[M20] + a[M13]*b[M30];
    result[M20] = a[M20]*b[M00] + a[M21]*b[M10] + a[M22]*b[M20] + a[M23]*b[M30];
    result[M30] = a[M30]*b[M00] + a[M31]*b[M10] + a[M32]*b[M20] + a[M33]*b[M30];
    result[M01] = a[M00]*b[M01] + a[M01]*b[M11] + a[M02]*b[M21] + a[M03]*b[M31];
    result[M11] = a[M10]*b[M01] + a[M11]*b[M11] + a[M12]*b[M21] + a[M13]*b[M31];
    result[M21] = a[M20]*b[M01] + a[M21]*b[M11] + a[M22]*b[M21] + a[M23]*b[M31];
    result[M31] = a[M30]*b[M01] + a[M31]*b[M11] + a[M32]*b[M21] + a[M33]*b[M31];
    result[M02] = a[M00]*b[M02] + a[M01]*b[M12] + a[M02]*b[M22] + a[M03]*b[M32];
    result[M12] = a[M10]*b[M02] + a[M11]*b[M12] + a[M12]*b[M22] + a[M13]*b[M32];
    result[M22] = a[M20]*b[M02] + a[M21]*b[M12] + a[M22]*b[M22] + a[M23]*b[M32];
    result[M32] = a[M30]*b[M02] + a[M31]*b[M12] + a[M32]*b[M22] + a[M33]*b[M32];
    result[M03] = a[M00]*b[M03] + a[M01]*b[M13] + a[M02]*b[M23] + a[M03]*b[M33];
    result[M13] = a[M10]*b[M03] + a[M11]*b[M13] + a[M12]*b[M23] + a[M13]*b[M33];
    result[M23] = a[M20]*b[M03] + a[M21]*b[M13] + a[M22]*b[M23] + a[M23]*b[M33];
    result[M33] = a[M30]*b[M03] + a[M31]*b[M13] + a[M32]*b[M23] + a[M33]*b[M33];
}

//If I hard-code all of the operations in-place, I probably don't need these two
//static inline void mat4_mul_into_a(mat4 *const a, const mat4 b)
//{
//    mat4 result = make_mat4_empty();
//    mat4_mul(*a, b, result);
//    free(*a);
//    *a = result;
//}
//
//static inline void mat4_mul_into_b(const mat4 a, mat4 *const b)
//{
//    mat4 result = make_mat4_empty();
//    mat4_mul(a, *b, result);
//    free(*b);
//    *b = result;
//}

//static inline void mat4_mul2(const mat4 a, const mat4 b, mat4 result)
//{
//    for (int aj = 0; aj < 4; ++aj)
//    {
//        for (int bi = 0; bi < 4; ++bi)
//        {
//            result[aj + bi * 4] = 0;
//            for (int n = 0; n < 4; ++n)
//            {
//                result[aj + bi * 4] += a[aj + n * 4] * b[n + bi * 4];
//            }
//        }
//    }
//}

static inline int mat4_equals(const mat4 a, const mat4 b)
{
    for (int i = 0; i < 16; ++i)
    {
        if (a[i] != b[i])
            return 0;
    }
    return 1;
}

//Debug: Print matrix in row-major form
static inline void mat4_print(const mat4 m)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            printf("%f ", m[j * 4 + i]);
        }
        printf("\n");
    }
    printf("\n");
}

static inline void free_mat4(mat4 *m)
{
    free(*m);
    *m = NULL;
}

////////////////////////////////////////////////////////////////////////////////

struct bbox {
    GLuint vao;
    GLuint vbos[2];
    struct program_bbox *program;
    struct vec4 vertices[8];
    //struct vec4 p0;
    //struct vec4 p1;

    //TODO: Make BBox shader program that takes color as uniform?
    struct col4 color;
};

void bbox_init(struct bbox *box);
void bbox_render(const struct bbox *box, mat4 model_matrix);
void bbox_render_color(const struct bbox *box, mat4 model_matrix,
                       const struct col4 color);

static inline struct bbox *make_bbox(struct vec4 p0, struct vec4 p1,
                                     struct col4 color)
{
    struct bbox *bbox = (struct bbox *)calloc(1, sizeof(struct bbox));
    bbox->program = make_program_bbox();
    bbox->vertices[0] = (struct vec4) { p0.x, p0.y, p0.z, 1.0f };
    bbox->vertices[1] = (struct vec4) { p1.x, p0.y, p0.z, 1.0f };
    bbox->vertices[2] = (struct vec4) { p1.x, p1.y, p0.z, 1.0f };
    bbox->vertices[3] = (struct vec4) { p0.x, p1.y, p0.z, 1.0f };
    bbox->vertices[4] = (struct vec4) { p0.x, p0.y, p1.z, 1.0f };
    bbox->vertices[5] = (struct vec4) { p1.x, p0.y, p1.z, 1.0f };
    bbox->vertices[6] = (struct vec4) { p1.x, p1.y, p1.z, 1.0f };
    bbox->vertices[7] = (struct vec4) { p0.x, p1.y, p1.z, 1.0f };
    bbox->color = color;
    return bbox;
}

static inline struct bbox *make_bbox_mesh(const struct vertex *verts,
                                          int count,
                                          struct col4 color)
{
    struct vec4 p0 = (struct vec4) { 9999.0f, 9999.0f, 9999.0f };
    struct vec4 p1 = (struct vec4) { -9999.0f, -9999.0f, -9999.0f };

    // Find bounds of mesh
    for (int i = 0; i < count; ++i)
    {
        if (verts[i].pos.x < p0.x)
            p0.x = verts[i].pos.x;
        else if (verts[i].pos.x > p1.x)
            p1.x = verts[i].pos.x;

        if (verts[i].pos.y < p0.y)
            p0.y = verts[i].pos.y;
        else if (verts[i].pos.y > p1.y)
            p1.y = verts[i].pos.y;

        if (verts[i].pos.z < p0.z)
            p0.z = verts[i].pos.z;
        else if (verts[i].pos.z > p1.z)
            p1.z = verts[i].pos.z;
    }

    // Prevent infinitesimally small bounds
    if (p0.x == p1.x)
    {
        p0.x -= BBOX_EPSILON;
        p1.x += BBOX_EPSILON;
    }
    if (p0.y == p1.y)
    {
        p0.y -= BBOX_EPSILON;
        p1.y += BBOX_EPSILON;
    }
    if (p0.z == p1.z)
    {
        p0.z -= BBOX_EPSILON;
        p1.z += BBOX_EPSILON;
    }

    return make_bbox(p0, p1, color);
}

static inline bool bbox_intersects(const struct bbox *a, const struct bbox *b)
{
    if (a->vertices[7].x < b->vertices[0].x) return false;
    if (b->vertices[7].x < a->vertices[0].x) return false;

    if (a->vertices[7].y < b->vertices[0].y) return false;
    if (b->vertices[7].y < a->vertices[0].y) return false;

    if (a->vertices[7].z < b->vertices[0].z) return false;
    if (b->vertices[7].z < a->vertices[0].z) return false;

    return true;
}

static inline void free_bbox(struct bbox **bbox)
{
    free(*bbox);
    *bbox = NULL;
}

////////////////////////////////////////////////////////////////////////////////

extern mat4 view_matrix;

////////////////////////////////////////////////////////////////////////////////

#endif