#ifndef GEOM_H
#define GEOM_H

#include "const.h"
#include <GL/gl3w.h>
#include <math.h>
#include <stdio.h>

#define VEC3_EPSILON 0.00001f
#define MAT4_EPSILON 0.00001f
#define QUAT_EPSILON 0.00001f

//------------------------------------------------------------------------------
// RGBA color
//------------------------------------------------------------------------------
struct col4 {
    float r, g, b, a;
};

//Predefined colors
extern const struct col4 COLOR_TRANSPARENT;
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

extern const struct col4 COLOR_DARK_RED_HIGHLIGHT;
extern const struct col4 COLOR_DARK_GREEN_HIGHLIGHT;
extern const struct col4 COLOR_DARK_BLUE_HIGHLIGHT;
extern const struct col4 COLOR_DARK_YELLOW_HIGHLIGHT;
extern const struct col4 COLOR_DARK_CYAN_HIGHLIGHT;
extern const struct col4 COLOR_DARK_MAGENTA_HIGHLIGHT;
extern const struct col4 COLOR_DARK_GRAY_HIGHLIGHT;

//------------------------------------------------------------------------------
// 2D Texture coordinates
//------------------------------------------------------------------------------
struct tex2 {
    float u, v;
};

//------------------------------------------------------------------------------
// 3D Vector
//------------------------------------------------------------------------------
struct vec3 {
    float x, y, z;
};

//------------------------------------------------------------------------------
// 4D Vector
//------------------------------------------------------------------------------
struct vec4 {
    float x, y, z, w;
};

extern const struct vec3 VEC3_ZERO;
extern const struct vec3 VEC3_ONE;
extern const struct vec3 VEC3_UNIT;
extern const struct vec3 VEC3_X;
extern const struct vec3 VEC3_Y;
extern const struct vec3 VEC3_Z;
extern const struct vec3 VEC3_UP;
extern const struct vec3 VEC3_FWD;
extern const struct vec3 VEC3_RIGHT;
extern const struct vec3 VEC3_SMALL;
extern const struct vec3 VEC3_SCALE_ASPECT;

static inline struct vec3 *vec3_add(struct vec3 *a, const struct vec3 *b)
{
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
    return a;
}
static inline struct vec3 *vec3_sub(struct vec3 *a, const struct vec3 *b)
{
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
    return a;
}
static inline struct vec3 *vec3_scale(struct vec3 *v, const struct vec3 *s)
{
    v->x *= s->x;
    v->y *= s->y;
    v->z *= s->z;
    return v;
}
static inline struct vec3 *vec3_scalef(struct vec3 *v, float s)
{
    v->x *= s;
    v->y *= s;
    v->z *= s;
    return v;
}
static inline float vec3_dot(const struct vec3 *a, const struct vec3 *b)
{
    return a->x*b->x + a->y*b->y + a->z*b->z;
}
static inline struct vec3 vec3_cross(const struct vec3 *a, const struct vec3 *b)
{
    struct vec3 c;
    c.x = a->y * b->z - a->z * b->y;
    c.y = a->z * b->x - a->x * b->z;
    c.z = a->x * b->y - a->y * b->x;
    return c;
}
static inline float vec3_length(const struct vec3 *v)
{
    return sqrtf(
        v->x * v->x +
        v->y * v->y +
        v->z * v->z
    );
}
static inline struct vec3 *vec3_negate(struct vec3 *v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
    return v;
}
static inline struct vec3 *vec3_normalize(struct vec3 *v)
{
    float len = 1.0f / vec3_length(v);
    v->x *= len;
    v->y *= len;
    v->z *= len;
    return v;
}
static inline struct vec3 *vec3_positive(struct vec3 *v)
{
    if (v->x < 0) v->x *= -1;
    if (v->y < 0) v->y *= -1;
    if (v->z < 0) v->z *= -1;
    return v;
}
static inline int vec3_equals(const struct vec3 *a, const struct vec3 *b)
{
    return (a->x == b->x && a->y == b->y && a->z == b->z);
}

static inline void vec3_print(struct vec3 *v)
{
    printf("Vec XYZ: %10f %10f %10f\n", v->x, v->y, v->z);
}

//------------------------------------------------------------------------------
// 4D Matrix
//------------------------------------------------------------------------------
struct mat4 {
    union {
        float a[16];    //Array
        float m[4][4];  //Matrix
        /*struct {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };*/
    };
};

extern const struct mat4 MAT4_IDENT;
extern const struct mat4 MAT4_PROJ_SCREEN;

//Store as row-major, one-dimensional array of floats
static inline struct mat4 mat4_init(
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

static inline struct mat4 mat4_init_transpose(const struct mat4 *m)
{
    return mat4_init(
        m->m[0][0], m->m[1][0], m->m[2][0], m->m[3][0],
        m->m[0][1], m->m[1][1], m->m[2][1], m->m[3][1],
        m->m[0][2], m->m[1][2], m->m[2][2], m->m[3][2],
        m->m[0][3], m->m[1][3], m->m[2][3], m->m[3][3]
    );
}

static inline struct mat4 mat4_init_translate(const struct vec3 *v)
{
    return mat4_init(
        1, 0, 0, v->x,
        0, 1, 0, v->y,
        0, 0, 1, v->z,
        0, 0, 0, 1
    );
}

static inline struct mat4 mat4_init_scale(const struct vec3 *s)
{
    return mat4_init(
        s->x, 0, 0, 0,
        0, s->y, 0, 0,
        0, 0, s->z, 0,
        0, 0, 0, 1
    );
}

static inline struct mat4 mat4_init_scalef(float s)
{
    return mat4_init(
        s, 0, 0, 0,
        0, s, 0, 0,
        0, 0, s, 0,
        0, 0, 0, 1
    );
}

static inline struct mat4 mat4_init_rotx(float deg)
{
    float rad = deg * M_PI / 180;
    float s = sinf(rad);
    float c = cosf(rad);
    return mat4_init(
        1, 0, 0, 0,
        0, c,-s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    );
}

static inline struct mat4 mat4_init_roty(float deg)
{
    float rad = deg * M_PI / 180;
    float s = sinf(rad);
    float c = cosf(rad);
    return mat4_init(
        c, 0, s, 0,
        0, 1, 0, 0,
       -s, 0, c, 0,
        0, 0, 0, 1
    );
}

static inline struct mat4 mat4_init_rotz(float deg)
{
    float rad = deg * M_PI / 180;
    float s = sinf(rad);
    float c = cosf(rad);
    return mat4_init(
        c,-s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

static inline int mat4_equals(const struct mat4 *a, const struct mat4 *b)
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

static struct mat4 *mat4_mul(struct mat4 *a, const struct mat4 *b)
{
    struct mat4 c = { 0 };
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

static inline void mat4_translate(struct mat4 *m, const struct vec3 *v)
{
    struct mat4 trans = mat4_init_translate(v);
    mat4_mul(m, &trans);
}

static inline void mat4_scale(struct mat4 *m, const struct vec3 *s)
{
    struct mat4 scale = mat4_init_scale(s);
    mat4_mul(m, &scale);
}

static inline void mat4_scalef(struct mat4 *m, float s)
{
    struct mat4 scale = mat4_init_scalef(s);
    mat4_mul(m, &scale);
}

static inline void mat4_rotx(struct mat4 *m, float deg)
{
    struct mat4 rot = mat4_init_rotx(deg);
    mat4_mul(m, &rot);
}

static inline void mat4_roty(struct mat4 *m, float deg)
{
    struct mat4 rot = mat4_init_roty(deg);
    mat4_mul(m, &rot);
}

static inline void mat4_rotz(struct mat4 *m, float deg)
{
    struct mat4 rot = mat4_init_rotz(deg);
    mat4_mul(m, &rot);
}

static inline void mat4_transpose(struct mat4 *m)
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

static inline struct vec3 *vec3_mul_mat4(struct vec3 *v, const struct mat4 *m)
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

//Calculate PERSPECTIVE projection
static inline struct mat4 mat4_init_perspective(float width, float height,
                                                float z_near, float z_far,
                                                float fov_deg)
{
    ///////////////////////////////////////////////////////

    float aspect = width / height;
    float dz = z_far - z_near;
    float fov_calc = 1.0f / tanf(DEG_TO_RAD(fov_deg) / 2.0f);

    struct mat4 mat = MAT4_IDENT;
    mat.m[0][0] = fov_calc / aspect;
    mat.m[1][1] = fov_calc;
    mat.m[2][2] = -(z_far + z_near) / dz;
    mat.m[2][3] = 2.0f * (z_far * z_near) / dz;
    mat.m[3][2] = -1.0f;
    return mat;
}

//Calculate lookAt matrix
static inline struct mat4 mat4_init_lookat(struct vec3 *pos, struct vec3 *view,
                                           struct vec3 *up)
{
    struct mat4 look;
    struct vec3 tmp = *view;
    struct vec3 right = vec3_cross(vec3_sub(&tmp, pos), up);
    vec3_normalize(&right);
    look = mat4_init(
        right.x, right.y, right.z, 0.0f,
          up->x,   up->y,   up->z, 0.0f,
        view->x, view->y, view->z, 0.0f,
           0.0f,    0.0f,    0.0f, 1.0f
    );
    struct mat4 trans = mat4_init_translate(vec3_negate(pos));
    mat4_mul(&look, &trans);
    //mat4_mul(&trans, &look);
    return look;
}

//Debug: Print matrix in row-major form
static inline void mat4_print(const struct mat4 *m)
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

//------------------------------------------------------------------------------
// Quaternion
//------------------------------------------------------------------------------
struct quat {
    float w;
    union {
        struct {
            float x, y, z;
        };
        struct vec3 v;
    };
};

const struct quat QUAT_IDENT;

static inline struct quat *quat_ident(struct quat *q)
{
    q->w = 1.0f;
    q->x = 0.0f;
    q->y = 0.0f;
    q->z = 0.0f;
    return q;
}

static inline int quat_is_ident(const struct quat *q)
{
    return (q->w == 1.0f &&
            q->x == 0.0f &&
            q->y == 0.0f &&
            q->z == 0.0f);
}

static inline int quat_equals(const struct quat *a, const struct quat *b)
{
    return (a->w == b->w &&
            a->x == b->x &&
            a->y == b->y &&
            a->z == b->z);
}

static inline float quat_norm_sq(const struct quat *q)
{
    return (q->w * q->w +
            q->x * q->x +
            q->y * q->y +
            q->z * q->z);
}

static inline float quat_norm(const struct quat *q)
{
    return sqrtf(quat_norm_sq(q));
}

static inline struct quat *quat_normalize(struct quat *q)
{
    float norm = quat_norm(q);

    // Quaternions of norm 1.0f are already normalized
    if (norm == 1.0f)
        return q;

    RICO_ASSERT(norm != 0.0f);
    q->w /= norm;
    q->x /= norm;
    q->y /= norm;
    q->z /= norm;
    return q;
    // TODO: Set 0,0,0,0 if len < epsilon
}

static inline struct quat *quat_conjugate(struct quat *q)
{
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
    return q;
    // TODO: Set 0,0,0,0 if len < epsilon
}

static inline struct quat *quat_inverse(struct quat *q)
{
    quat_conjugate(q);
    float norm_sq = quat_norm_sq(q);

    // Inverse == conjugate for normalized ("unit-norm") quaternions
    if (norm_sq == 1.0f)
        return q;

    RICO_ASSERT(norm_sq != 0.0f);
    q->w /= norm_sq;
    q->x /= norm_sq;
    q->y /= norm_sq;
    q->z /= norm_sq;
    return q;
    // TODO: Set 0,0,0,0 if len < epsilon
}

//static inline struct quat *quat_mul(struct quat *a, const struct quat *b)
static inline struct quat *quat_mul(struct quat *a, const struct quat *b)
{
    struct quat c;
    c.w = a->w*b->w - a->x*b->x - a->y*b->y - a->z*b->z;
    c.x = a->w*b->x + a->x*b->w + a->y*b->z - a->z*b->y;
    c.y = a->w*b->y - a->x*b->z + a->y*b->w + a->z*b->x;
    c.z = a->w*b->z + a->x*b->y - a->y*b->x + a->z*b->w;
    *a = c;
    return a;
}

static inline float quat_dot(const struct quat *a, const struct quat *b)
{
    return (a->w * b->w +
            a->x * b->x +
            a->y * b->y +
            a->z * b->z);
}

static inline struct quat *quat_from_axis_angle(struct quat *q,
                                                const struct vec3 *axis,
                                                float angle_deg)
{
    float s = sinf(DEG_TO_RAD(angle_deg) / 2.0f);
    q->w = cosf(DEG_TO_RAD(angle_deg) / 2.0f);
    q->x = axis->x * s;
    q->y = axis->y * s;
    q->z = axis->z * s;

    quat_normalize(q);
    return q;
}

static inline struct vec3 *vec3_mul_quat(struct vec3 *v, const struct quat *q)
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

    if (fabs(qq.w) < QUAT_EPSILON) qq.w = 0.0f;
    if (fabs(qq.x) < QUAT_EPSILON) qq.x = 0.0f;
    if (fabs(qq.y) < QUAT_EPSILON) qq.y = 0.0f;
    if (fabs(qq.z) < QUAT_EPSILON) qq.z = 0.0f;

    // Quaternion must be pure to properly convert back into vec3
    RICO_ASSERT(qq.w == 0.0f);
    v->x = qq.x;
    v->y = qq.y;
    v->z = qq.z;
    return v;
}

////////////////////////////////////////////////////////////////////////////////
// I don't think these are useful quaternion operations.
////////////////////////////////////////////////////////////////////////////////
static inline struct quat *quat_scale(struct quat *q, float s)
{
    q->w *= s;
    q->x *= s;
    q->y *= s;
    q->z *= s;
    return q;
}

static inline struct quat *quat_add(struct quat *a, struct quat *b)
{
    a->w += b->w;
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
    return a;
}

static inline struct quat *quat_sub(struct quat *a, struct quat *b)
{
    a->w -= b->w;
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
    return a;
}
////////////////////////////////////////////////////////////////////////////////

static inline void mat4_from_quat(struct mat4 *_m, const struct quat *q)
{
    struct quat qq = *q;
    quat_normalize(&qq);

    float a = qq.x;
    float b = qq.y;
    float c = qq.z;
    float d = qq.w;

    // This code assumes quaternion is normalized
    _m->m[0][0] = 1 - 2*b*b - 2*c*c;
    _m->m[0][1] = 2*a*b - 2*c*d;
    _m->m[0][2] = 2*a*c + 2*b*d;
    _m->m[0][3] = 0;
    _m->m[1][0] = 2*a*b + 2*c*d;
    _m->m[1][1] = 1 - 2*a*a - 2*c*c;
    _m->m[1][2] = 2*b*c - 2*a*d;
    _m->m[1][3] = 0;
    _m->m[2][0] = 2*a*c - 2*b*d;
    _m->m[2][1] = 2*b*c + 2*a*d;
    _m->m[2][2] = 1 - 2*a*a - 2*b*b;
    _m->m[2][3] = 0;
    _m->m[3][0] = 0;
    _m->m[3][1] = 0;
    _m->m[3][2] = 0;
    _m->m[3][3] = 1;

    // _m->m[0][0] = a*a + b*b - c*c - d*d;
    // _m->m[1][0] = 2*b*c - 2*a*d;
    // _m->m[2][0] = 2*b*d + 2*a*c;
    // _m->m[3][0] = 0;
    // _m->m[0][1] = 2*b*c + 2*a*d;
    // _m->m[1][1] = a*a - b*b + c*c - d*d;
    // _m->m[2][1] = 2*c*d - 2*a*b;
    // _m->m[3][1] = 0;
    // _m->m[0][2] = 2*b*d - 2*a*c;
    // _m->m[1][2] = 2*c*d + 2*a*b;
    // _m->m[2][2] = a*a - b*b - c*c + d*d;
    // _m->m[3][2] = 0;
    // _m->m[0][3] = 0;
    // _m->m[1][3] = 0;
    // _m->m[2][3] = 0;
    // _m->m[3][3] = a*a + b*b + c*c + d*d;
}

static inline void quat_print(struct quat *q)
{
    printf("Quat WXYZ: %10f %10f %10f %10f\n", q->w, q->x, q->y, q->z);
}

/*
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

*/

//------------------------------------------------------------------------------
// Ray
//------------------------------------------------------------------------------
struct ray {
    struct vec3 orig;
    struct vec3 dir;
};

//------------------------------------------------------------------------------
// Sphere
//------------------------------------------------------------------------------
struct sphere {
    struct vec3 orig;
    float radius;
};

#endif // GEOM_H