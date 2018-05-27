//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//
// #define DLB_MATH_IMPLEMENTATION
//      <math.h>
//
// #define DLB_MATH_PRINT
//      <stdio.h>
//
// #include "dlb_math.h"
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_MATH_H
#define DLB_MATH_H

#include "dlb_types.h"
#include <math.h>
#ifdef DLB_MATH_PRINT
    #include <stdio.h>
#endif

#ifdef DLB_MATH_EXTERN
    #define DLB_MATH_DEF extern
#else
    #define DLB_MATH_DEF static inline
#endif

#define M_PI 3.14159265358979323846264338327950288
#define M_2PI 6.28318530717958647692528676655900576

#define DEG_TO_RAD(deg) deg * M_PI / 180.0
#define RAD_TO_DEG(rad) rad * 180.0 / M_PI
#define DEG_TO_RADF(deg) deg * (float)M_PI / 180.0f
#define RAD_TO_DEGF(rad) rad * 180.0f / (float)M_PI

#define LERP(v0, v1, t) (1 - t) * v0 + t * v1

#define KB(bytes) (1024 * bytes)
#define MB(bytes) (1024 * KB(bytes))
#define GB(bytes) (1024 * MB(bytes))

#define MATH_EPSILON 0.00001f
#define VEC3_EPSILON MATH_EPSILON
#define MAT4_EPSILON MATH_EPSILON
#define QUAT_EPSILON MATH_EPSILON

struct vec2
{
    union
    {
        struct { float x, y; };
        struct { float u, v; };
    };
};

struct vec3
{
    union
    {
        struct { float x, y, z; };
        struct { float u, v, w; };
        struct { float r, g, b; };
        float a[3];
    };
};

struct vec4
{
    union
    {
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
    };
};

struct mat4
{
    union
    {
        float a[16];    //Array
        float m[4][4];  //Matrix
        /*struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };*/
    };
};

struct quat
{
    float w;
    union
    {
        struct
        {
            float x, y, z;
        };
        struct vec3 v;
    };
};

// HACK: Make gcc shut up about missing braces bug. While this syntax is
//       technically valid in VS, it makes Intellisense whine and misbehave.
#if defined(__GNUC__) || defined(__clang__) // || defined(_MSC_VER)
#define VEC2(x, y)       ((const struct vec2) {{{ x, y }}})
#define VEC3(x, y, z)    ((const struct vec3) {{{ x, y, z }}})
#define VEC4(x, y, z, w) ((const struct vec4) {{{ x, y, z, w }}})
#define QUAT(w, x, y, z) ((const struct quat) { w, {{ x, y, z }}})
#else
#define VEC2(x, y)       ((const struct vec2) { x, y })
#define VEC3(x, y, z)    ((const struct vec3) { x, y, z })
#define VEC4(x, y, z, w) ((const struct vec4) { x, y, z, w })
#define QUAT(w, x, y, z) ((const struct quat) { w, x, y, z })
#endif

#define COLOR_TRANSPARENT             VEC4(0.000f, 0.000f, 0.000f, 0.0f)

#define COLOR_ORANGE                  VEC4(1.000f, 0.549f, 0.000f, 1.0f)
#define COLOR_PINK                    VEC4(0.933f, 0.510f, 0.933f, 1.0f)
#define COLOR_PURPLE                  VEC4(0.541f, 0.169f, 0.886f, 1.0f)
#define COLOR_LIME                    VEC4(0.678f, 1.000f, 0.184f, 1.0f)
#define COLOR_AQUA                    VEC4(0.400f, 0.804f, 0.667f, 1.0f)
#define COLOR_DODGER                   VEC4(0.118f, 0.565f, 1.000f, 1.0f)
#define COLOR_WHEAT                   VEC4(0.961f, 0.871f, 0.702f, 1.0f)
#define COLOR_BROWN                   VEC4(0.545f, 0.271f, 0.075f, 1.0f)

#define COLOR_BLACK                   VEC4(0.000f, 0.000f, 0.000f, 1.0f)
#define COLOR_RED                     VEC4(1.000f, 0.000f, 0.000f, 1.0f)
#define COLOR_GREEN                   VEC4(0.000f, 1.000f, 0.000f, 1.0f)
#define COLOR_BLUE                    VEC4(0.000f, 0.000f, 1.000f, 1.0f)
#define COLOR_YELLOW                  VEC4(1.000f, 1.000f, 0.000f, 1.0f)
#define COLOR_CYAN                    VEC4(0.000f, 1.000f, 1.000f, 1.0f)
#define COLOR_MAGENTA                 VEC4(1.000f, 0.000f, 1.000f, 1.0f)
#define COLOR_WHITE                   VEC4(1.000f, 1.000f, 1.000f, 1.0f)

#define COLOR_DARK_RED                VEC4(0.500f, 0.000f, 0.000f, 1.0f)
#define COLOR_DARK_GREEN              VEC4(0.000f, 0.500f, 0.000f, 1.0f)
#define COLOR_DARK_BLUE               VEC4(0.000f, 0.000f, 0.500f, 1.0f)
#define COLOR_DARK_YELLOW             VEC4(0.500f, 0.500f, 0.000f, 1.0f)
#define COLOR_DARK_CYAN               VEC4(0.000f, 0.500f, 0.500f, 1.0f)
#define COLOR_DARK_MAGENTA            VEC4(0.500f, 0.000f, 0.500f, 1.0f)
#define COLOR_DARK_WHITE              VEC4(0.500f, 0.500f, 0.500f, 1.0f)

#define COLOR_DARK_RED_HIGHLIGHT      VEC4(0.500f, 0.000f, 0.000f, 0.5f)
#define COLOR_DARK_GREEN_HIGHLIGHT    VEC4(0.000f, 0.500f, 0.000f, 0.5f)
#define COLOR_DARK_BLUE_HIGHLIGHT     VEC4(0.000f, 0.000f, 0.500f, 0.5f)
#define COLOR_DARK_YELLOW_HIGHLIGHT   VEC4(0.500f, 0.500f, 0.000f, 0.5f)
#define COLOR_DARK_CYAN_HIGHLIGHT     VEC4(0.000f, 0.500f, 0.500f, 0.5f)
#define COLOR_DARK_MAGENTA_HIGHLIGHT  VEC4(0.500f, 0.000f, 0.500f, 0.5f)
#define COLOR_DARK_WHITE_HIGHLIGHT    VEC4(0.500f, 0.500f, 0.500f, 0.5f)

#define VEC3_ZERO                     VEC3(0.0f, 0.0f, 0.0f)
#define VEC3_ONE                      VEC3(1.0f, 1.0f, 1.0f)
#define VEC3_UNIT                     VEC3(0.577350259f, 0.577350259f, 0.577350259f)
#define VEC3_X                        VEC3(1.0f, 0.0f, 0.0f)
#define VEC3_Y                        VEC3(0.0f, 1.0f, 0.0f)
#define VEC3_Z                        VEC3(0.0f, 0.0f, 1.0f)
#define VEC3_RIGHT                    VEC3(1.0f, 0.0f, 0.0f)
#define VEC3_UP                       VEC3(0.0f, 1.0f, 0.0f)
#define VEC3_FWD                      VEC3(0.0f, 0.0f,-1.0f)
#define VEC3_DOWN                     VEC3(0.0f,-1.0f, 0.0f)
#define VEC3_SMALL                    VEC3(0.01f, 0.01f, 0.01f)

#define QUAT_IDENT QUAT(1.0f, 0.0f, 0.0f, 0.0f)

static const struct mat4 MAT4_IDENT;

//--- Vectors --------------------------
DLB_MATH_DEF struct vec3 *v3_add(struct vec3 *a, const struct vec3 *b);
DLB_MATH_DEF struct vec3 *v3_sub(struct vec3 *a, const struct vec3 *b);
DLB_MATH_DEF struct vec3 *v3_scale(struct vec3 *v, const struct vec3 *s);
DLB_MATH_DEF struct vec3 *v3_scalef(struct vec3 *v, float s);
DLB_MATH_DEF float v3_dot(const struct vec3 *a, const struct vec3 *b);
DLB_MATH_DEF struct vec3 v3_cross(const struct vec3 *a, const struct vec3 *b);
DLB_MATH_DEF float v3_length(const struct vec3 *v);
DLB_MATH_DEF struct vec3 *v3_negate(struct vec3 *v);
DLB_MATH_DEF struct vec3 *v3_normalize(struct vec3 *v);
DLB_MATH_DEF struct vec3 *v3_positive(struct vec3 *v);
DLB_MATH_DEF int v3_equals(const struct vec3 *a, const struct vec3 *b);
DLB_MATH_DEF struct vec3 *v3_mul_mat4(struct vec3 *v, const struct mat4 *m);
DLB_MATH_DEF struct vec3 *v3_mul_quat(struct vec3 *v, const struct quat *q);

//--- Matrices -------------------------
DLB_MATH_DEF struct mat4 mat4_init(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33);
DLB_MATH_DEF struct mat4 mat4_init_transpose(const struct mat4 *m);
DLB_MATH_DEF struct mat4 mat4_init_translate(const struct vec3 *v);
DLB_MATH_DEF struct mat4 mat4_init_scale(const struct vec3 *s);
DLB_MATH_DEF struct mat4 mat4_init_scalef(float s);
DLB_MATH_DEF struct mat4 mat4_init_rotx(float deg);
DLB_MATH_DEF struct mat4 mat4_init_roty(float deg);
DLB_MATH_DEF struct mat4 mat4_init_rotz(float deg);
DLB_MATH_DEF struct mat4 mat4_init_quat(const struct quat *q);
DLB_MATH_DEF int mat4_equals(const struct mat4 *a, const struct mat4 *b);
DLB_MATH_DEF struct mat4 *mat4_mul(struct mat4 *a, const struct mat4 *b);
DLB_MATH_DEF void mat4_translate(struct mat4 *m, const struct vec3 *v);
DLB_MATH_DEF void mat4_scale(struct mat4 *m, const struct vec3 *s);
DLB_MATH_DEF void mat4_scalef(struct mat4 *m, float s);
DLB_MATH_DEF void mat4_rotx(struct mat4 *m, float deg);
DLB_MATH_DEF void mat4_roty(struct mat4 *m, float deg);
DLB_MATH_DEF void mat4_rotz(struct mat4 *m, float deg);
DLB_MATH_DEF void mat4_transpose(struct mat4 *m);
DLB_MATH_DEF struct mat4 mat4_init_perspective(float width, float height,
                                               float z_near, float z_far,
                                               float fov_deg);
DLB_MATH_DEF struct mat4 mat4_init_lookat(struct vec3 *pos, struct vec3 *view,
                                          struct vec3 *up);

//--- Quaternions ----------------------
DLB_MATH_DEF struct quat *quat_ident(struct quat *q);
DLB_MATH_DEF int quat_is_ident(const struct quat *q);
DLB_MATH_DEF int quat_equals(const struct quat *a, const struct quat *b);
DLB_MATH_DEF float quat_norm_sq(const struct quat *q);
DLB_MATH_DEF float quat_norm(const struct quat *q);
DLB_MATH_DEF struct quat *quat_normalize(struct quat *q);
DLB_MATH_DEF struct quat *quat_conjugate(struct quat *q);
DLB_MATH_DEF struct quat *quat_inverse(struct quat *q);
DLB_MATH_DEF struct quat *quat_mul(struct quat *a, const struct quat *b);
DLB_MATH_DEF float quat_dot(const struct quat *a, const struct quat *b);
DLB_MATH_DEF struct quat *quat_from_axis_angle(struct quat *q, struct vec3 axis,
                                               float angle_deg);

// I don't think these are useful quaternion operations...
DLB_MATH_DEF struct quat *quat_scale(struct quat *q, float s);
DLB_MATH_DEF struct quat *quat_add(struct quat *a, struct quat *b);
DLB_MATH_DEF struct quat *quat_sub(struct quat *a, struct quat *b);

#if 0
// https://github.com/Kazade/kazmath/blob/master/kazmath/quaternion.c
// https://github.com/dagostinelli/hypatia/blob/master/src/quaternion.c
// http://www.3dgep.com/understanding-quaternions/#Quaternion_Norm

quat quat_ident()
int quat_is_ident(q)
int quat_are_equal(a, b)
float quat_norm(q)
float quat_norm_sq(q)
quat quat_normalize(q)  // Set 0,0,0,0 if len < epsilon
quat quat_conj(q)       // Set 0,0,0,0 if len < epsilon
quat quat_inv(q)        // Set 0,0,0,0 if len < epsilon
quat quat_mul(a, b)
float quat_dot(a, b)
quat quat_scale(q, float s)
quat quat_add(a, b)
quat quat_sub(a, b)

quat quat_rot_axis(q, vec3 v, float angle)  // Rotate axis using quaternion
quat quat_from_mat(m)
quat quat_from_ypr(float yaw, float pitch, float roll)
quat quat_slerp(a, b, float t)
void quat_to_axis(q, vec3 *v, float *angle)
vec3 quat_mul_vec(q, v)
vec3 quat_to_up(q)  // Return UP vector rotated by this quaternion
vec3 quat_to_right(q)
vec3 quat_to_fwd(q)
vec3 quat_to_fwd_lh(q) // Left-handed forward (+Z)
float quat_yaw(q)
float quat_pitch(q)
float quat_roll(q)
quat quat_between_vec(vec3 u, vec3 v)

quat quat_around_axis(q, vec3 axis) // Component of quaternion around axis
quat quat_lookat(vec3 fwd, vec3 up) // Calculate lookAt quaternion
quat quat_exp(q) // "Exponential"
quat quat_ln(q) // "Natural logarithm"
quat quat_between_vec(vec3 a, vec3 b, vec3 fallback) // ????
#endif

//--- Miscellaneous --------------------
struct ray
{
    struct vec3 orig;
    struct vec3 dir;
};

struct quad
{
    // (0,0) (0,1) (1,0) (1,1)
    struct vec3 verts[4];
};

struct sphere
{
    struct vec3 orig;
    float radius;
};

#ifdef DLB_MATH_PRINT
DLB_MATH_DEF void v3_print(struct vec3 *v);
DLB_MATH_DEF void mat4_print(const struct mat4 *m);
DLB_MATH_DEF void quat_print(struct quat *q);
#endif

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_MATH_IMPLEMENTATION
#if 1
static const struct mat4 MAT4_IDENT = {{{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
}}};

DLB_MATH_DEF struct vec3 *v3_add(struct vec3 *a, const struct vec3 *b)
{
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
    return a;
}
DLB_MATH_DEF struct vec3 *v3_sub(struct vec3 *a, const struct vec3 *b)
{
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
    return a;
}
DLB_MATH_DEF struct vec3 *v3_scale(struct vec3 *v, const struct vec3 *s)
{
    v->x *= s->x;
    v->y *= s->y;
    v->z *= s->z;
    return v;
}
DLB_MATH_DEF struct vec3 *v3_scalef(struct vec3 *v, float s)
{
    v->x *= s;
    v->y *= s;
    v->z *= s;
    return v;
}
DLB_MATH_DEF float v3_dot(const struct vec3 *a, const struct vec3 *b)
{
    float dot = a->x*b->x + a->y*b->y + a->z*b->z;
    if (fabsf(dot) < VEC3_EPSILON) dot = 0.0f;
    return dot;
}
DLB_MATH_DEF struct vec3 v3_cross(const struct vec3 *a, const struct vec3 *b)
{
    struct vec3 c;
    c.x = a->y * b->z - a->z * b->y;
    c.y = a->z * b->x - a->x * b->z;
    c.z = a->x * b->y - a->y * b->x;
    return c;
}
DLB_MATH_DEF float v3_length(const struct vec3 *v)
{
    return sqrtf(
        v->x * v->x +
        v->y * v->y +
        v->z * v->z
    );
}
DLB_MATH_DEF struct vec3 *v3_negate(struct vec3 *v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
    return v;
}
DLB_MATH_DEF struct vec3 *v3_normalize(struct vec3 *v)
{
    float len = v3_length(v);
    if (len == 0) return v;

    len = 1.0f / len;
    v->x *= len;
    v->y *= len;
    v->z *= len;
    return v;
}
DLB_MATH_DEF struct vec3 *v3_positive(struct vec3 *v)
{
    if (v->x < 0) v->x *= -1;
    if (v->y < 0) v->y *= -1;
    if (v->z < 0) v->z *= -1;
    return v;
}
DLB_MATH_DEF int v3_equals(const struct vec3 *a, const struct vec3 *b)
{
    return (a->x == b->x && a->y == b->y && a->z == b->z);
}
DLB_MATH_DEF struct vec3 *v3_mul_mat4(struct vec3 *v, const struct mat4 *m)
{
    // Copy v
    struct vec3 vv = *v;

    // Multiply m * v
    v->x = m->m[0][0] * vv.x +
        m->m[0][1] * vv.y +
        m->m[0][2] * vv.z +
        m->m[0][3] * 1.f;
    v->y = m->m[1][0] * vv.x +
        m->m[1][1] * vv.y +
        m->m[1][2] * vv.z +
        m->m[1][3] * 1.f;
    v->z = m->m[2][0] * vv.x +
        m->m[2][1] * vv.y +
        m->m[2][2] * vv.z +
        m->m[2][3] * 1.f;
    return v;
}
DLB_MATH_DEF struct vec3 *v3_mul_quat(struct vec3 *v, const struct quat *q)
{
    // Copy q
    struct quat qq = *q;

    // Create pure quaternion from v
    struct quat qv;
    qv.w = 0.0f;
    qv.x = v->x;
    qv.y = v->y;
    qv.z = v->z;

    // Rotate v by q
    quat_mul(quat_mul(quat_conjugate(&qq), &qv), q);
    /* USE THIS INSTEAD !!
    quat_mul(quat_conjugate(&qq), &qv);
    quat_mul(&qq, q);
    */

    if (fabsf(qq.w) < QUAT_EPSILON) qq.w = 0.0f;
    if (fabsf(qq.x) < QUAT_EPSILON) qq.x = 0.0f;
    if (fabsf(qq.y) < QUAT_EPSILON) qq.y = 0.0f;
    if (fabsf(qq.z) < QUAT_EPSILON) qq.z = 0.0f;

    // Quaternion must be pure to properly convert back into vec3
    assert(qq.w == 0.0f);
    v->x = qq.x;
    v->y = qq.y;
    v->z = qq.z;
    return v;
}
#else
DLB_MATH_DEF struct vec3 v3_add(struct vec3 a, struct vec3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}
DLB_MATH_DEF struct vec3 v3_sub(struct vec3 a, struct vec3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}
DLB_MATH_DEF struct vec3 v3_scale(struct vec3 v, struct vec3 s)
{
    v.x *= s.x;
    v.y *= s.y;
    v.z *= s.z;
    return v;
}
DLB_MATH_DEF struct vec3 v3_scalef(struct vec3 v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}
DLB_MATH_DEF float v3_dot(struct vec3 a, struct vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
DLB_MATH_DEF struct vec3 v3_cross(struct vec3 a, struct vec3 b)
{
    struct vec3 c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;
    return c;
}
DLB_MATH_DEF float v3_length(struct vec3 v)
{
    return sqrtf(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
    );
}
DLB_MATH_DEF struct vec3 v3_negate(struct vec3 v)
{
    v.x = -v.x;
    v.y = -v.y;
    v.z = -v.z;
    return v;
}
DLB_MATH_DEF struct vec3 v3_normalize(struct vec3 v)
{
    float len = 1.0f / v3_length(v);
    v.x *= len;
    v.y *= len;
    v.z *= len;
    return v;
}
DLB_MATH_DEF struct vec3 v3_positive(struct vec3 v)
{
    v.x = fabsf(v.x);
    v.y = fabsf(v.y);
    v.z = fabsf(v.z);
    return v;
}
DLB_MATH_DEF int v3_equals(struct vec3 a, struct vec3 b)
{
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}
#endif

//Store as row-major, one-dimensional array of floats
DLB_MATH_DEF struct mat4 mat4_init(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33)
{
    struct mat4 mat;
    mat.m[0][0] = m00;
    mat.m[0][1] = m01;
    mat.m[0][2] = m02;
    mat.m[0][3] = m03;
    mat.m[1][0] = m10;
    mat.m[1][1] = m11;
    mat.m[1][2] = m12;
    mat.m[1][3] = m13;
    mat.m[2][0] = m20;
    mat.m[2][1] = m21;
    mat.m[2][2] = m22;
    mat.m[2][3] = m23;
    mat.m[3][0] = m30;
    mat.m[3][1] = m31;
    mat.m[3][2] = m32;
    mat.m[3][3] = m33;
    return mat;
}

DLB_MATH_DEF struct mat4 mat4_init_transpose(const struct mat4 *m)
{
    return mat4_init(
        m->m[0][0], m->m[1][0], m->m[2][0], m->m[3][0],
        m->m[0][1], m->m[1][1], m->m[2][1], m->m[3][1],
        m->m[0][2], m->m[1][2], m->m[2][2], m->m[3][2],
        m->m[0][3], m->m[1][3], m->m[2][3], m->m[3][3]
    );
}

DLB_MATH_DEF struct mat4 mat4_init_translate(const struct vec3 *v)
{
    return mat4_init(
        1, 0, 0, v->x,
        0, 1, 0, v->y,
        0, 0, 1, v->z,
        0, 0, 0, 1
    );
}

DLB_MATH_DEF struct mat4 mat4_init_scale(const struct vec3 *s)
{
    return mat4_init(
        s->x, 0, 0, 0,
        0, s->y, 0, 0,
        0, 0, s->z, 0,
        0, 0, 0, 1
    );
}

DLB_MATH_DEF struct mat4 mat4_init_scalef(float s)
{
    return mat4_init(
        s, 0, 0, 0,
        0, s, 0, 0,
        0, 0, s, 0,
        0, 0, 0, 1
    );
}

DLB_MATH_DEF struct mat4 mat4_init_rotx(float deg)
{
    float rad = DEG_TO_RADF(deg);
    float s = sinf(rad);
    float c = cosf(rad);
    return mat4_init(
        1, 0, 0, 0,
        0, c,-s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    );
}

DLB_MATH_DEF struct mat4 mat4_init_roty(float deg)
{
    float rad = DEG_TO_RADF(deg);
    float s = sinf(rad);
    float c = cosf(rad);
    return mat4_init(
        c, 0, s, 0,
        0, 1, 0, 0,
       -s, 0, c, 0,
        0, 0, 0, 1
    );
}

DLB_MATH_DEF struct mat4 mat4_init_rotz(float deg)
{
    float rad = DEG_TO_RADF(deg);
    float s = sinf(rad);
    float c = cosf(rad);
    return mat4_init(
        c,-s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

DLB_MATH_DEF struct mat4 mat4_init_quat(const struct quat *q)
{
    struct quat qq = *q;
    quat_normalize(&qq);

    float a = qq.x;
    float b = qq.y;
    float c = qq.z;
    float d = qq.w;

    struct mat4 m = { 0 };
    m.m[0][0] = 1 - 2*b*b - 2*c*c;
    m.m[0][1] = 2*a*b - 2*c*d;
    m.m[0][2] = 2*a*c + 2*b*d;
    m.m[0][3] = 0;
    m.m[1][0] = 2*a*b + 2*c*d;
    m.m[1][1] = 1 - 2*a*a - 2*c*c;
    m.m[1][2] = 2*b*c - 2*a*d;
    m.m[1][3] = 0;
    m.m[2][0] = 2*a*c - 2*b*d;
    m.m[2][1] = 2*b*c + 2*a*d;
    m.m[2][2] = 1 - 2*a*a - 2*b*b;
    m.m[2][3] = 0;
    m.m[3][0] = 0;
    m.m[3][1] = 0;
    m.m[3][2] = 0;
    m.m[3][3] = 1;
    return m;
}

DLB_MATH_DEF int mat4_equals(const struct mat4 *a, const struct mat4 *b)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (fabsf(a->m[i][j] - b->m[i][j]) >= MAT4_EPSILON)
                return 0;
        }
    }
    return 1;
}

DLB_MATH_DEF struct mat4 *mat4_mul(struct mat4 *a, const struct mat4 *b)
{
    struct mat4 c = {{{ 0 }}};
    for (int aj = 0; aj < 4; ++aj)
    {
        for (int bi = 0; bi < 4; ++bi)
        {
            for (int n = 0; n < 4; ++n)
            {
                c.m[aj][bi] += a->m[aj][n] * b->m[n][bi];
            }
        }
    }
    *a = c;
    return a;
}

DLB_MATH_DEF void mat4_translate(struct mat4 *m, const struct vec3 *v)
{
    struct mat4 trans = mat4_init_translate(v);
    mat4_mul(m, &trans);
}

DLB_MATH_DEF void mat4_scale(struct mat4 *m, const struct vec3 *s)
{
    struct mat4 scale = mat4_init_scale(s);
    mat4_mul(m, &scale);
}

DLB_MATH_DEF void mat4_scalef(struct mat4 *m, float s)
{
    struct mat4 scale = mat4_init_scalef(s);
    mat4_mul(m, &scale);
}

DLB_MATH_DEF void mat4_rotx(struct mat4 *m, float deg)
{
    struct mat4 rot = mat4_init_rotx(deg);
    mat4_mul(m, &rot);
}

DLB_MATH_DEF void mat4_roty(struct mat4 *m, float deg)
{
    struct mat4 rot = mat4_init_roty(deg);
    mat4_mul(m, &rot);
}

DLB_MATH_DEF void mat4_rot_quat(struct mat4 *m, struct quat *q)
{
    struct mat4 rot = mat4_init_quat(q);
    mat4_mul(m, &rot);
}

DLB_MATH_DEF void mat4_rotz(struct mat4 *m, float deg)
{
    struct mat4 rot = mat4_init_rotz(deg);
    mat4_mul(m, &rot);
}

DLB_MATH_DEF void mat4_transpose(struct mat4 *m)
{
    float tmp;

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
DLB_MATH_DEF struct mat4 mat4_init_perspective(float width, float height,
                                               float z_near, float z_far,
                                               float fov_deg)
{
    float aspect = width / height;
    float dz = z_far - z_near;
    float fov_calc = 1.0f / tanf(DEG_TO_RADF(fov_deg) / 2.0f);

    struct mat4 mat = MAT4_IDENT;
    mat.m[0][0] = fov_calc / aspect;
    mat.m[1][1] = fov_calc;
    mat.m[2][2] = -(z_far + z_near) / dz;
    mat.m[2][3] = 2.0f * (z_far * z_near) / dz;
    mat.m[3][2] = -1.0f;
    return mat;
}

//Calculate ORTHOGRAPHIC projection
DLB_MATH_DEF struct mat4 mat4_init_ortho(float width, float height,
                                         float z_near, float z_far)
{
    float aspect = width / height;
    float dz = z_far - z_near;

    struct mat4 mat = MAT4_IDENT;
    mat.m[0][0] = 1.0f / aspect;
    mat.m[1][1] = 1.0f;
    mat.m[2][2] = -2.0f / dz;
    mat.m[2][3] = (z_far + z_near) / dz;
    mat.m[3][3] = 500.0f;
    return mat;
}

//Calculate lookAt matrix
DLB_MATH_DEF struct mat4 mat4_init_lookat(struct vec3 *pos, struct vec3 *view,
                                          struct vec3 *up)
{
    struct mat4 look;
    struct vec3 right = v3_cross(v3_sub(view, pos), up);
    v3_normalize(&right);
    look = mat4_init(
        right.x,  right.y, right.z, 0.0f,
          up->x,    up->y,   up->z, 0.0f,
        view->x,  view->y, view->z, 0.0f,
           0.0f,    0.0f,    0.0f, 1.0f
    );
    struct mat4 trans = mat4_init_translate(v3_negate(pos));
    mat4_mul(&look, &trans);
    //mat4_mul(&trans, &look);
    return look;
}

DLB_MATH_DEF struct quat *quat_ident(struct quat *q)
{
    q->w = 1.0f;
    q->x = 0.0f;
    q->y = 0.0f;
    q->z = 0.0f;
    return q;
}

DLB_MATH_DEF int quat_is_ident(const struct quat *q)
{
    return (q->w == 1.0f &&
            q->x == 0.0f &&
            q->y == 0.0f &&
            q->z == 0.0f);
}

DLB_MATH_DEF int quat_equals(const struct quat *a, const struct quat *b)
{
    return (a->w == b->w &&
            a->x == b->x &&
            a->y == b->y &&
            a->z == b->z);
}

DLB_MATH_DEF float quat_norm_sq(const struct quat *q)
{
    return (q->w * q->w +
            q->x * q->x +
            q->y * q->y +
            q->z * q->z);
}

DLB_MATH_DEF float quat_norm(const struct quat *q)
{
    return sqrtf(quat_norm_sq(q));
}

DLB_MATH_DEF struct quat *quat_normalize(struct quat *q)
{
    float norm = quat_norm(q);

    // Quaternions of norm 1.0f are already normalized
    if (norm == 1.0f)
        return q;

    assert(norm != 0.0f);
    q->w /= norm;
    q->x /= norm;
    q->y /= norm;
    q->z /= norm;
    return q;
    // TODO: Set 0,0,0,0 if len < epsilon
}

DLB_MATH_DEF struct quat *quat_conjugate(struct quat *q)
{
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
    return q;
    // TODO: Set 0,0,0,0 if len < epsilon
}

DLB_MATH_DEF struct quat *quat_inverse(struct quat *q)
{
    quat_conjugate(q);
    float norm_sq = quat_norm_sq(q);

    // Inverse == conjugate for normalized ("unit-norm") quaternions
    if (norm_sq == 1.0f)
        return q;

    assert(norm_sq != 0.0f);
    q->w /= norm_sq;
    q->x /= norm_sq;
    q->y /= norm_sq;
    q->z /= norm_sq;
    return q;
    // TODO: Set 0,0,0,0 if len < epsilon
}

//DLB_MATH_DEF struct quat *quat_mul(struct quat *a, const struct quat *b)
DLB_MATH_DEF struct quat *quat_mul(struct quat *a, const struct quat *b)
{
    struct quat c;
    c.w = a->w*b->w - a->x*b->x - a->y*b->y - a->z*b->z;
    c.x = a->w*b->x + a->x*b->w + a->y*b->z - a->z*b->y;
    c.y = a->w*b->y - a->x*b->z + a->y*b->w + a->z*b->x;
    c.z = a->w*b->z + a->x*b->y - a->y*b->x + a->z*b->w;
    *a = c;
    return a;
}

DLB_MATH_DEF float quat_dot(const struct quat *a, const struct quat *b)
{
    return (a->w * b->w +
            a->x * b->x +
            a->y * b->y +
            a->z * b->z);
}

DLB_MATH_DEF struct quat *quat_from_axis_angle(struct quat *q, struct vec3 axis,
                                               float angle_deg)
{
    float s = sinf(DEG_TO_RADF(angle_deg) / 2.0f);
    q->w = cosf(DEG_TO_RADF(angle_deg) / 2.0f);
    q->x = axis.x * s;
    q->y = axis.y * s;
    q->z = axis.z * s;

    quat_normalize(q);
    return q;
}

////////////////////////////////////////////////////////////////////////////////
// I don't think these are useful quaternion operations.
////////////////////////////////////////////////////////////////////////////////
DLB_MATH_DEF struct quat *quat_scale(struct quat *q, float s)
{
    q->w *= s;
    q->x *= s;
    q->y *= s;
    q->z *= s;
    return q;
}

DLB_MATH_DEF struct quat *quat_add(struct quat *a, struct quat *b)
{
    a->w += b->w;
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
    return a;
}

DLB_MATH_DEF struct quat *quat_sub(struct quat *a, struct quat *b)
{
    a->w -= b->w;
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
    return a;
}
////////////////////////////////////////////////////////////////////////////////

#ifdef DLB_MATH_PRINT
#include <stdio.h>
DLB_MATH_DEF void v3_print(struct vec3 *v)
{
    printf("Vec XYZ: %10f %10f %10f\n", v->x, v->y, v->z);
}

// Debug: Print matrix in row-major form
DLB_MATH_DEF void mat4_print(const struct mat4 *m)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            printf("%f ", m->m[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

DLB_MATH_DEF void quat_print(struct quat *q)
{
    printf("Quat WXYZ: %10f %10f %10f %10f\n", q->w, q->x, q->y, q->z);
}
#endif

#endif
