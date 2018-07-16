#include "chet_collision.h"
#include "dlb_types.h"
#include "math.h"

// 3D OBB v. OBB collision test using separating-axis theorem
// Based on Christer Ericson's book:
//   "Real-Time Collision Detection" (p. 103)
//   http://realtimecollisiondetection.net/books/rtcd/
// and David Eberly's paper:
//   "Dynamic Collision Detection using Oriented Bounding Boxes" (p. 7)
//   https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
int obb_v_obb(struct RICO_obb *a, struct RICO_obb *b)
{
    bool a_has_obb = v3_length(&a->e) > 0.0f;
    bool b_has_obb = v3_length(&b->e) > 0.0f;
    if (!a_has_obb || !b_has_obb)
        return -1;
    DLB_ASSERT(a_has_obb);
    DLB_ASSERT(b_has_obb);

    // TODO: Find and return collision manifold

#define ABSS(x) ((x) > 0 ? (x) : (-x))
    float aaa1 = ABS(1.0f);
    float aaa2 = ABS(-1.0f);

    UNUSED(aaa1);
    UNUSED(aaa2);

    float ra, rb;
    struct mat4 C = { 0 };
    struct mat4 CAbs = { 0 };

    struct vec3 t = b->c;
    v3_sub(&t, &a->c);
    t = VEC3(v3_dot(&t, &a->u[0]), v3_dot(&t, &a->u[1]), v3_dot(&t, &a->u[2]));

    // Test axes A0, A1, A2
    // (and build B -> A transform matrix for AABB v. OBB optimization)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            C.m[i][j] = v3_dot(&a->u[i], &b->u[j]);
            CAbs.m[i][j] = fabsf(C.m[i][j]) + MATH_EPSILON;
        }

        ra = a->e.a[i];
        rb = b->e.a[0] * CAbs.m[i][0] + b->e.a[1] * CAbs.m[i][1] + b->e.a[2] * CAbs.m[i][2];
        if (fabsf(t.a[i]) > ra + rb) {
            return i + 1;
        }
    }

    // Test axes B0, B1, B2
    for (int i = 0; i < 3; ++i) {
        ra = a->e.a[0] * CAbs.m[0][i] + a->e.a[1] * CAbs.m[1][i] + a->e.a[2] * CAbs.m[2][i];
        rb = b->e.a[i];
        if (fabsf(t.a[0] * C.m[0][i] + t.a[1] * C.m[1][i] + t.a[2] * C.m[2][i]) > ra + rb) {
            return i + 4;
        }
    }

    // Test axis L = A0 x B0
    ra = a->e.a[1] * CAbs.m[2][0] + a->e.a[2] * CAbs.m[1][0];
    rb = b->e.a[1] * CAbs.m[0][2] + b->e.a[2] * CAbs.m[0][1];
    if (fabsf(t.a[2] * C.m[1][0] - t.a[1] * C.m[2][0]) > ra + rb) {
        return 7;
    }
    // Test axis L = A0 x B1
    ra = a->e.a[1] * CAbs.m[2][1] + a->e.a[2] * CAbs.m[1][1];
    rb = b->e.a[0] * CAbs.m[0][2] + b->e.a[2] * CAbs.m[0][0];
    if (fabsf(t.a[2] * C.m[1][1] - t.a[1] * C.m[2][1]) > ra + rb) {
        return 8;
    }
    // Test axis L = A0 x B2
    ra = a->e.a[1] * CAbs.m[2][2] + a->e.a[2] * CAbs.m[1][2];
    rb = b->e.a[0] * CAbs.m[0][1] + b->e.a[1] * CAbs.m[0][0];
    if (fabsf(t.a[2] * C.m[1][2] - t.a[1] * C.m[2][2]) > ra + rb) {
        return 9;
    }
    // Test axis L = A1 x B0
    ra = a->e.a[0] * CAbs.m[2][0] + a->e.a[2] * CAbs.m[0][0];
    rb = b->e.a[1] * CAbs.m[1][2] + b->e.a[2] * CAbs.m[1][1];
    if (fabsf(t.a[0] * C.m[2][0] - t.a[2] * C.m[0][0]) > ra + rb) {
        return 10;
    }
    // Test axis L = A1 x B1
    ra = a->e.a[0] * CAbs.m[2][1] + a->e.a[2] * CAbs.m[0][1];
    rb = b->e.a[0] * CAbs.m[1][2] + b->e.a[2] * CAbs.m[1][0];
    if (fabsf(t.a[0] * C.m[2][1] - t.a[2] * C.m[0][1]) > ra + rb) {
        return 11;
    }
    // Test axis L = A1 x B2
    ra = a->e.a[0] * CAbs.m[2][2] + a->e.a[2] * CAbs.m[0][2];
    rb = b->e.a[0] * CAbs.m[1][1] + b->e.a[1] * CAbs.m[1][0];
    if (fabsf(t.a[0] * C.m[2][2] - t.a[2] * C.m[0][2]) > ra + rb) {
        return 12;
    }
    // Test axis L = A2 x B0
    ra = a->e.a[0] * CAbs.m[1][0] + a->e.a[1] * CAbs.m[0][0];
    rb = b->e.a[1] * CAbs.m[2][2] + b->e.a[2] * CAbs.m[2][1];
    if (fabsf(t.a[1] * C.m[0][0] - t.a[0] * C.m[1][0]) > ra + rb) {
        return 13;
    }
    // Test axis L = A2 x B1
    ra = a->e.a[0] * CAbs.m[1][1] + a->e.a[1] * CAbs.m[0][1];
    rb = b->e.a[0] * CAbs.m[2][2] + b->e.a[2] * CAbs.m[2][0];
    if (fabsf(t.a[1] * C.m[0][1] - t.a[0] * C.m[1][1]) > ra + rb) {
        return 14;
    }
    // Test axis L = A2 x B2
    ra = a->e.a[0] * CAbs.m[1][2] + a->e.a[1] * CAbs.m[0][2];
    rb = b->e.a[0] * CAbs.m[2][1] + b->e.a[1] * CAbs.m[2][0];
    if (fabsf(t.a[1] * C.m[0][2] - t.a[0] * C.m[1][2]) > ra + rb) {
        return 15;
    }

    return 0;
}

// Based on David Eberly's paper:
// "Dynamic Collision Detection using Oriented Bounding Boxes" (p. 7)
// https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
int obb_v_obb_eberly(struct RICO_obb *a, struct RICO_obb *b)
{
    float R, R0, R1;
    struct vec3 D = b->c;
    v3_sub(&D, &a->c);

    struct mat4 C = MAT4_IDENT;
    struct mat4 AbsC = MAT4_IDENT;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            C.m[i][j] = v3_dot(&a->u[i], &b->u[j]);
            AbsC.m[i][j] = fabsf(C.m[i][j]) + MATH_EPSILON;
        }
    }

#define A0 &a->u[0]
#define A1 &a->u[1]
#define A2 &a->u[2]
#define B0 &b->u[0]
#define B1 &b->u[1]
#define B2 &b->u[2]
#define a0 a->e.x
#define a1 a->e.y
#define a2 a->e.z
#define b0 b->e.x
#define b1 b->e.y
#define b2 b->e.z
#define c00 C.m[0][0]
#define c01 C.m[0][1]
#define c02 C.m[0][2]
#define c10 C.m[1][0]
#define c11 C.m[1][1]
#define c12 C.m[1][2]
#define c20 C.m[2][0]
#define c21 C.m[2][1]
#define c22 C.m[2][2]
#define abs_c00 AbsC.m[0][0]
#define abs_c01 AbsC.m[0][1]
#define abs_c02 AbsC.m[0][2]
#define abs_c10 AbsC.m[1][0]
#define abs_c11 AbsC.m[1][1]
#define abs_c12 AbsC.m[1][2]
#define abs_c20 AbsC.m[2][0]
#define abs_c21 AbsC.m[2][1]
#define abs_c22 AbsC.m[2][2]

    ////////

    R0 = a0;
    R1 = b0*abs_c00 + b1*abs_c01 + b2*abs_c02;
    R = fabsf(v3_dot(A0, &D));
    if (R > R0 + R1)
        return true;

    R0 = a1;
    R1 = b0*abs_c10 + b1*abs_c11 + b2*abs_c12;
    R = fabsf(v3_dot(A1, &D));
    if (R > R0 + R1)
        return true;

    R0 = a2;
    R1 = b0*abs_c20 + b1*abs_c21 + b2*abs_c22;
    R = fabsf(v3_dot(A2, &D));
    if (R > R0 + R1)
        return true;

    ////////

    R0 = b0;
    R1 = a0*abs_c00 + a1*abs_c10 + a2*abs_c20;
    R = fabsf(v3_dot(B0, &D));
    if (R > R0 + R1)
        return true;

    R0 = b1;
    R1 = a0*abs_c01 + a1*abs_c11 + a2*abs_c21;
    R = fabsf(v3_dot(B1, &D));
    if (R > R0 + R1)
        return true;

    R0 = b2;
    R1 = a0*abs_c02 + a1*abs_c12 + a2*abs_c22;
    R = fabsf(v3_dot(B2, &D));
    if (R > R0 + R1)
        return true;

    ////////////////////////////////////////////////////////////////////////////
    struct vec3 t0, t1;
    ////////////////////////////////////////////////////////////////////////////

    R0 = a1*abs_c20 + a2*abs_c10;
    R1 = b1*abs_c02 + b2*abs_c01;
    t0 = *A2;
    t1 = *A1;
    R = fabsf(v3_dot(v3_scalef(&t0, c10), &D) -
              v3_dot(v3_scalef(&t1, c20), &D));
    if (R > R0 + R1)
        return true;

    R0 = a1*abs_c21 + a2*abs_c11;
    R1 = b0*abs_c02 + b2*abs_c00;
    t0 = *A2;
    t1 = *A1;
    R = fabsf(v3_dot(v3_scalef(&t0, c11), &D) -
              v3_dot(v3_scalef(&t1, c21), &D));
    if (R > R0 + R1)
        return true;

    R0 = a1*abs_c22 + a2*abs_c12;
    R1 = b0*abs_c01 + b1*abs_c00;
    t0 = *A2;
    t1 = *A1;
    R = fabsf(v3_dot(v3_scalef(&t0, c12), &D) -
              v3_dot(v3_scalef(&t1, c22), &D));
    if (R > R0 + R1)
        return true;

    ////////////////////////////////////////////////////////////////////////////

    R0 = a0*abs_c20 + a2*abs_c00;
    R1 = b1*abs_c12 + b2*abs_c11;
    t0 = *A0;
    t1 = *A2;
    R = fabsf(v3_dot(v3_scalef(&t0, c20), &D) -
              v3_dot(v3_scalef(&t1, c00), &D));
    if (R > R0 + R1)
        return true;

    R0 = a0*abs_c21 + a2*abs_c01;
    R1 = b0*abs_c12 + b2*abs_c10;
    t0 = *A0;
    t1 = *A2;
    R = fabsf(v3_dot(v3_scalef(&t0, c21), &D) -
              v3_dot(v3_scalef(&t1, c01), &D));
    if (R > R0 + R1)
        return true;

    R0 = a0*abs_c22 + a2*abs_c02;
    R1 = b0*abs_c11 + b1*abs_c10;
    t0 = *A0;
    t1 = *A2;
    R = fabsf(v3_dot(v3_scalef(&t0, c22), &D) -
              v3_dot(v3_scalef(&t1, c02), &D));
    if (R > R0 + R1)
        return true;

    ////////////////////////////////////////////////////////////////////////////

    R0 = a0*abs_c10 + a1*abs_c00;
    R1 = b1*abs_c22 + b2*abs_c21;
    t0 = *A1;
    t1 = *A0;
    R = fabsf(v3_dot(v3_scalef(&t0, c00), &D) -
              v3_dot(v3_scalef(&t1, c10), &D));
    if (R > R0 + R1)
        return true;

    R0 = a0*abs_c11 + a1*abs_c01;
    R1 = b0*abs_c22 + b2*abs_c20;
    t0 = *A1;
    t1 = *A0;
    R = fabsf(v3_dot(v3_scalef(&t0, c01), &D) -
              v3_dot(v3_scalef(&t1, c11), &D));
    if (R > R0 + R1)
        return true;

    R0 = a0*abs_c12 + a1*abs_c02;
    R1 = b0*abs_c21 + b1*abs_c20;
    t0 = *A1;
    t1 = *A0;
    R = fabsf(v3_dot(v3_scalef(&t0, c02), &D) -
              v3_dot(v3_scalef(&t1, c12), &D));
    if (R > R0 + R1)
        return true;

#undef A0
#undef A1
#undef A2
#undef B0
#undef B1
#undef B2
#undef a0
#undef a1
#undef a2
#undef b0
#undef b1
#undef b2
#undef c00
#undef c01
#undef c02
#undef c10
#undef c11
#undef c12
#undef c20
#undef c21
#undef c22
#undef abs_c00
#undef abs_c01
#undef abs_c02
#undef abs_c10
#undef abs_c11
#undef abs_c12
#undef abs_c20
#undef abs_c21
#undef abs_c22

    return false;
}