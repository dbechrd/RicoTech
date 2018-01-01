#ifndef GEOM_H
#define GEOM_H

#define VEC3_EPSILON 0.00001f
#define MAT4_EPSILON 0.00001f
#define QUAT_EPSILON 0.00001f

#define VEC2(a, b) ((struct vec2) {{{ a, b }}})
#define VEC3(a, b, c) ((struct vec3) {{{ a, b, c }}})
#define VEC4(a, b, c, d) ((struct vec4) {{{ a, b, c, d }}})

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

#if 1
internal inline struct vec3 *v3_add(struct vec3 *a, const struct vec3 *b)
{
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
    return a;
}
internal inline struct vec3 *v3_sub(struct vec3 *a, const struct vec3 *b)
{
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
    return a;
}
internal inline struct vec3 *v3_scale(struct vec3 *v, const struct vec3 *s)
{
    v->x *= s->x;
    v->y *= s->y;
    v->z *= s->z;
    return v;
}
internal inline struct vec3 *v3_scalef(struct vec3 *v, float s)
{
    v->x *= s;
    v->y *= s;
    v->z *= s;
    return v;
}
internal inline float v3_dot(const struct vec3 *a, const struct vec3 *b)
{
    return a->x*b->x + a->y*b->y + a->z*b->z;
}
internal inline struct vec3 v3_cross(const struct vec3 *a, const struct vec3 *b)
{
    struct vec3 c;
    c.x = a->y * b->z - a->z * b->y;
    c.y = a->z * b->x - a->x * b->z;
    c.z = a->x * b->y - a->y * b->x;
    return c;
}
internal inline float v3_length(const struct vec3 *v)
{
    return sqrtf(
        v->x * v->x +
        v->y * v->y +
        v->z * v->z
    );
}
internal inline struct vec3 *v3_negate(struct vec3 *v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
    return v;
}
internal inline struct vec3 *v3_normalize(struct vec3 *v)
{
    float len = v3_length(v);
    if (len == 0) return v;

    len = 1.0f / len;
    v->x *= len;
    v->y *= len;
    v->z *= len;
    return v;
}
internal inline struct vec3 *v3_positive(struct vec3 *v)
{
    if (v->x < 0) v->x *= -1;
    if (v->y < 0) v->y *= -1;
    if (v->z < 0) v->z *= -1;
    return v;
}
internal inline int v3_equals(const struct vec3 *a, const struct vec3 *b)
{
    return (a->x == b->x && a->y == b->y && a->z == b->z);
}

// TODO: Refactor *all* printf stuff out into a single file
internal inline void v3_print(struct vec3 *v)
{
    printf("Vec XYZ: %10f %10f %10f\n", v->x, v->y, v->z);
}
#else
internal inline struct vec3 v3_add(struct vec3 a, struct vec3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}
internal inline struct vec3 v3_sub(struct vec3 a, struct vec3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}
internal inline struct vec3 v3_scale(struct vec3 v, struct vec3 s)
{
    v.x *= s.x;
    v.y *= s.y;
    v.z *= s.z;
    return v;
}
internal inline struct vec3 v3_scalef(struct vec3 v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}
internal inline float v3_dot(struct vec3 a, struct vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
internal inline struct vec3 v3_cross(struct vec3 a, struct vec3 b)
{
    struct vec3 c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;
    return c;
}
internal inline float v3_length(struct vec3 v)
{
    return sqrtf(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
    );
}
internal inline struct vec3 v3_negate(struct vec3 v)
{
    v.x = -v.x;
    v.y = -v.y;
    v.z = -v.z;
    return v;
}
internal inline struct vec3 v3_normalize(struct vec3 v)
{
    float len = 1.0f / v3_length(v);
    v.x *= len;
    v.y *= len;
    v.z *= len;
    return v;
}
internal inline struct vec3 v3_positive(struct vec3 v)
{
    v.x = fabsf(v.x);
    v.y = fabsf(v.y);
    v.z = fabsf(v.z);
    return v;
}
internal inline int v3_equals(struct vec3 a, struct vec3 b)
{
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

// TODO: Refactor *all* printf stuff out into a single file
internal inline void v3_print(struct vec3 v)
{
    printf("Vec XYZ: %10f %10f %10f\n", v.x, v.y, v.z);
}
#endif

//------------------------------------------------------------------------------
// 4D Matrix
//------------------------------------------------------------------------------
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

extern const struct mat4 MAT4_IDENT;
//extern const struct mat4 MAT4_PROJ_SCREEN;

//Store as row-major, one-dimensional array of floats
internal inline struct mat4 mat4_init(
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

internal inline struct mat4 mat4_init_transpose(const struct mat4 *m)
{
    return mat4_init(
        m->m[0][0], m->m[1][0], m->m[2][0], m->m[3][0],
        m->m[0][1], m->m[1][1], m->m[2][1], m->m[3][1],
        m->m[0][2], m->m[1][2], m->m[2][2], m->m[3][2],
        m->m[0][3], m->m[1][3], m->m[2][3], m->m[3][3]
    );
}

internal inline struct mat4 mat4_init_translate(const struct vec3 *v)
{
    return mat4_init(
        1, 0, 0, v->x,
        0, 1, 0, v->y,
        0, 0, 1, v->z,
        0, 0, 0, 1
    );
}

internal inline struct mat4 mat4_init_scale(const struct vec3 *s)
{
    return mat4_init(
        s->x, 0, 0, 0,
        0, s->y, 0, 0,
        0, 0, s->z, 0,
        0, 0, 0, 1
    );
}

internal inline struct mat4 mat4_init_scalef(float s)
{
    return mat4_init(
        s, 0, 0, 0,
        0, s, 0, 0,
        0, 0, s, 0,
        0, 0, 0, 1
    );
}

internal inline struct mat4 mat4_init_rotx(float deg)
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

internal inline struct mat4 mat4_init_roty(float deg)
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

internal inline struct mat4 mat4_init_rotz(float deg)
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

internal inline int mat4_equals(const struct mat4 *a, const struct mat4 *b)
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

internal inline struct mat4 *mat4_mul(struct mat4 *a, const struct mat4 *b)
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

internal inline void mat4_translate(struct mat4 *m, const struct vec3 *v)
{
    struct mat4 trans = mat4_init_translate(v);
    mat4_mul(m, &trans);
}

internal inline void mat4_scale(struct mat4 *m, const struct vec3 *s)
{
    struct mat4 scale = mat4_init_scale(s);
    mat4_mul(m, &scale);
}

internal inline void mat4_scalef(struct mat4 *m, float s)
{
    struct mat4 scale = mat4_init_scalef(s);
    mat4_mul(m, &scale);
}

internal inline void mat4_rotx(struct mat4 *m, float deg)
{
    struct mat4 rot = mat4_init_rotx(deg);
    mat4_mul(m, &rot);
}

internal inline void mat4_roty(struct mat4 *m, float deg)
{
    struct mat4 rot = mat4_init_roty(deg);
    mat4_mul(m, &rot);
}

internal inline void mat4_rotz(struct mat4 *m, float deg)
{
    struct mat4 rot = mat4_init_rotz(deg);
    mat4_mul(m, &rot);
}

internal inline void mat4_transpose(struct mat4 *m)
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

internal inline struct vec3 *v3_mul_mat4(struct vec3 *v, const struct mat4 *m)
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
internal inline struct mat4 mat4_init_perspective(float width, float height,
                                                  float z_near, float z_far,
                                                  float fov_deg)
{
    ///////////////////////////////////////////////////////

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

//Calculate lookAt matrix
internal inline struct mat4 mat4_init_lookat(struct vec3 *pos,
                                             struct vec3 *view,
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

//Debug: Print matrix in row-major form
internal inline void mat4_print(const struct mat4 *m)
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

const struct quat QUAT_IDENT;

internal inline struct quat *quat_ident(struct quat *q)
{
    q->w = 1.0f;
    q->x = 0.0f;
    q->y = 0.0f;
    q->z = 0.0f;
    return q;
}

internal inline int quat_is_ident(const struct quat *q)
{
    return (q->w == 1.0f &&
            q->x == 0.0f &&
            q->y == 0.0f &&
            q->z == 0.0f);
}

internal inline int quat_equals(const struct quat *a, const struct quat *b)
{
    return (a->w == b->w &&
            a->x == b->x &&
            a->y == b->y &&
            a->z == b->z);
}

internal inline float quat_norm_sq(const struct quat *q)
{
    return (q->w * q->w +
            q->x * q->x +
            q->y * q->y +
            q->z * q->z);
}

internal inline float quat_norm(const struct quat *q)
{
    return sqrtf(quat_norm_sq(q));
}

internal inline struct quat *quat_normalize(struct quat *q)
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

internal inline struct quat *quat_conjugate(struct quat *q)
{
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
    return q;
    // TODO: Set 0,0,0,0 if len < epsilon
}

internal inline struct quat *quat_inverse(struct quat *q)
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

//internal inline struct quat *quat_mul(struct quat *a, const struct quat *b)
internal inline struct quat *quat_mul(struct quat *a, const struct quat *b)
{
    struct quat c;
    c.w = a->w*b->w - a->x*b->x - a->y*b->y - a->z*b->z;
    c.x = a->w*b->x + a->x*b->w + a->y*b->z - a->z*b->y;
    c.y = a->w*b->y - a->x*b->z + a->y*b->w + a->z*b->x;
    c.z = a->w*b->z + a->x*b->y - a->y*b->x + a->z*b->w;
    *a = c;
    return a;
}

internal inline float quat_dot(const struct quat *a, const struct quat *b)
{
    return (a->w * b->w +
            a->x * b->x +
            a->y * b->y +
            a->z * b->z);
}

internal inline struct quat *quat_from_axis_angle(struct quat *q,
                                                  struct vec3 axis,
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

internal inline struct vec3 *v3_mul_quat(struct vec3 *v, const struct quat *q)
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
    RICO_ASSERT(qq.w == 0.0f);
    v->x = qq.x;
    v->y = qq.y;
    v->z = qq.z;
    return v;
}

////////////////////////////////////////////////////////////////////////////////
// I don't think these are useful quaternion operations.
////////////////////////////////////////////////////////////////////////////////
internal inline struct quat *quat_scale(struct quat *q, float s)
{
    q->w *= s;
    q->x *= s;
    q->y *= s;
    q->z *= s;
    return q;
}

internal inline struct quat *quat_add(struct quat *a, struct quat *b)
{
    a->w += b->w;
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
    return a;
}

internal inline struct quat *quat_sub(struct quat *a, struct quat *b)
{
    a->w -= b->w;
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
    return a;
}
////////////////////////////////////////////////////////////////////////////////

internal inline void mat4_from_quat(struct mat4 *_m, const struct quat *q)
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

internal inline void quat_print(struct quat *q)
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
struct ray
{
    struct vec3 orig;
    struct vec3 dir;
};

//------------------------------------------------------------------------------
// Sphere
//------------------------------------------------------------------------------
struct sphere
{
    struct vec3 orig;
    float radius;
};

#endif // GEOM_H