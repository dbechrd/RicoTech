#ifndef CHET_OBB_H
#define CHET_OBB_H

#include "dlb_types.h"

bool obb_v_obb(const struct RICO_obb *a, const struct RICO_obb *b)
{
    float ra, rb;
    struct mat4 R, AbsR;

    DLB_ASSERT(v3_length(&a->e) > 0.0f);

    // Rotation matrix of b in a's coordinate space
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            R.m[i][j] = v3_dot(&a->u[i], &b->u[j]);
        }
    }

    struct vec3 t = b->c;
    v3_sub(&t, &a->c);
    t = VEC3(v3_dot(&t, &a->u[0]), v3_dot(&t, &a->u[1]), v3_dot(&t, &a->u[2]));

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            AbsR.m[i][j] = ABS(R.m[i][j]) + 0.01f;
        }
    }

    // Test axes A0, A1, A2
    for (int i = 0; i < 3; ++i) {
        ra = a->e.a[i];
        rb = b->e.a[0] * AbsR.m[i][0] + b->e.a[1] * AbsR.m[i][1] + b->e.a[2] * AbsR.m[i][2];
        if (ABS(t.a[i]) > ra + rb) {
            return false;
        }
    }

    // Test axes B0, B1, B2
    for (int i = 0; i < 3; ++i) {
        ra = a->e.a[0] * AbsR.m[0][i] + a->e.a[1] * AbsR.m[1][i] + a->e.a[2] * AbsR.m[2][i];
        rb = b->e.a[i];
        if (ABS(t.a[0] * R.m[0][i] + t.a[1] * R.m[1][i] + t.a[2] * R.m[2][i]) > ra + rb) {
            return false;
        }
    }

    // Test axis L = A0 x B0
    ra = a->e.a[1] * AbsR.m[2][0] + a->e.a[2] * AbsR.m[1][0];
    rb = b->e.a[1] * AbsR.m[0][2] + b->e.a[2] * AbsR.m[0][1];
    if (ABS(t.a[2] * R.m[1][0] - t.a[1] * R.m[2][0]) > ra + rb) {
        return false;
    }
    // Test axis L = A0 x B1
    ra = a->e.a[1] * AbsR.m[2][1] + a->e.a[2] * AbsR.m[1][1];
    rb = b->e.a[0] * AbsR.m[0][2] + b->e.a[2] * AbsR.m[0][0];
    if (ABS(t.a[2] * R.m[1][1] - t.a[1] * R.m[2][1]) > ra + rb) {
        return false;
    }
    // Test axis L = A0 x B2
    ra = a->e.a[1] * AbsR.m[2][2] + a->e.a[2] * AbsR.m[1][2];
    rb = b->e.a[0] * AbsR.m[0][1] + b->e.a[1] * AbsR.m[0][0];
    if (ABS(t.a[2] * R.m[1][2] - t.a[1] * R.m[2][2]) > ra + rb) {
        return false;
    }
    // Test axis L = A1 x B0
    ra = a->e.a[0] * AbsR.m[2][0] + a->e.a[2] * AbsR.m[0][0];
    rb = b->e.a[1] * AbsR.m[1][2] + b->e.a[2] * AbsR.m[1][1];
    if (ABS(t.a[0] * R.m[2][0] - t.a[2] * R.m[0][0]) > ra + rb) {
        return false;
    }
    // Test axis L = A1 x B1
    ra = a->e.a[0] * AbsR.m[2][1] + a->e.a[2] * AbsR.m[0][1];
    rb = b->e.a[0] * AbsR.m[1][2] + b->e.a[2] * AbsR.m[1][0];
    if (ABS(t.a[0] * R.m[2][1] - t.a[2] * R.m[0][1]) > ra + rb) {
        return false;
    }
    // Test axis L = A1 x B2
    ra = a->e.a[0] * AbsR.m[2][2] + a->e.a[2] * AbsR.m[0][2];
    rb = b->e.a[0] * AbsR.m[1][1] + b->e.a[1] * AbsR.m[1][0];
    if (ABS(t.a[0] * R.m[2][2] - t.a[2] * R.m[0][2]) > ra + rb) {
        return false;
    }
    // Test axis L = A2 x B0
    ra = a->e.a[0] * AbsR.m[1][0] + a->e.a[1] * AbsR.m[0][0];
    rb = b->e.a[1] * AbsR.m[2][2] + b->e.a[2] * AbsR.m[2][1];
    if (ABS(t.a[1] * R.m[0][0] - t.a[0] * R.m[1][0]) > ra + rb) {
        return false;
    }
    // Test axis L = A2 x B1
    ra = a->e.a[0] * AbsR.m[1][1] + a->e.a[1] * AbsR.m[0][1];
    rb = b->e.a[0] * AbsR.m[2][2] + b->e.a[2] * AbsR.m[2][0];
    if (ABS(t.a[1] * R.m[0][1] - t.a[0] * R.m[1][1]) > ra + rb) {
        return false;
    }
    // Test axis L = A2 x B2
    ra = a->e.a[0] * AbsR.m[1][2] + a->e.a[1] * AbsR.m[0][2];
    rb = b->e.a[0] * AbsR.m[2][1] + b->e.a[1] * AbsR.m[2][0];
    if (ABS(t.a[1] * R.m[0][2] - t.a[0] * R.m[1][2]) > ra + rb) {
        return false;
    }

    return true;
}

#endif