#include "test_geom.h"

void test_geom()
{
    //TODO: Use Unity test framework (http://www.throwtheswitch.org/unity)
    //http://www.drdobbs.com/testing/unit-testing-in-c-tools-and-conventions/240156344
    //run_tests();

    ////////////////////////////////////////////////////////////////////////////

    //// Test translate / scale order
    //mat4 scale = mat4_scale((struct vec4) { 10.0f, 11.0f, 12.0f, 1.0f });
    //mat4 trans = mat4_translate((struct vec4) { 2.f, 3.f, 4.f, 1.f });
    //mat4 result = make_mat4_empty();
    //
    //printf("Trans * Scale\n");
    //mat4_mul(trans, scale, result);
    //mat4_print(result);

    struct mat4 a = make_mat4(
        3.f, 2.f, 9.f, 6.f,
        9.f, 6.f, 3.f, 5.f,
        9.f, 7.f, 6.f, 5.f,
        1.f, 5.f, 3.f, 3.f
    );

    struct mat4 b = make_mat4(
        5.f, 1.f, 3.f, 2.f,
        1.f, 6.f, 4.f, 3.f,
        9.f, 2.f, 1.f, 4.f,
        7.f, 5.f, 5.f, 3.f
    );

    //--------------------------------------------------------------------------
    //// Test old and new translate
    //struct vec4 trans = (struct vec4) { 2.0f, 3.0f, 4.0f };
    //mat4 b = make_mat4_translate(trans);
    struct mat4 result = { 0 };
    //
    mat4_mul(&a, &b, &result);
    //mat4_print(result);
    //
    //mat4_translate(a, trans);
    //mat4_print(a);
    //printf("translate valid: %i\n\n", mat4_equals(a, result));

    //--------------------------------------------------------------------------
    //// Test old and new scale
    //struct vec4 scale = (struct vec4) { 2.0f, 3.0f, 4.0f };
    //mat4 b = make_mat4_scale(scale);
    //mat4 result = make_mat4_empty();
    //
    //mat4_mul(a, b, result);
    //mat4_print(result);
    //
    //mat4_scale(a, scale);
    //mat4_print(a);
    //printf("scale valid: %i\n\n", mat4_equals(a, result));

    //--------------------------------------------------------------------------
    //// Test rot x
    //mat4 b = make_mat4_rotx(18);
    //mat4 result = make_mat4_empty();
    //
    //mat4_mul(a, b, result);
    //mat4_print(result);
    //
    //mat4_rotx(a, 18);
    //mat4_print(a);
    //printf("rot x valid: %i\n\n", mat4_equals(a, result));

    //--------------------------------------------------------------------------
    //// Test rot y
    //mat4 b = make_mat4_roty(18);
    //mat4 result = make_mat4_empty();

    //mat4_mul(a, b, result);
    //mat4_print(result);

    //mat4_roty(a, 18);
    //mat4_print(a);
    //printf("rot y valid: %i\n\n", mat4_equals(a, result));

    //--------------------------------------------------------------------------
    //// Test rot z
    //mat4 b = make_mat4_rotz(18);
    //mat4 result = make_mat4_empty();

    //mat4_mul(a, b, result);
    //mat4_print(result);

    //mat4_rotz(a, 18);
    //mat4_print(a);
    //printf("rot z valid: %i\n\n", mat4_equals(a, result));

    ////////////////////////////////////////////////////////////////////////////
}