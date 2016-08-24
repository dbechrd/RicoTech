#include "test_geom.h"

void test_geom()
{
    ////////////////////////////////////////////////////////////////////////////

    //// Test translate / scale order
    //mat5 scale = mat5_scale((struct vec4) { 10.0f, 11.0f, 12.0f, 1.0f });
    //mat5 trans = mat5_translate((struct vec4) { 2.f, 3.f, 4.f, 1.f });
    //mat5 result = make_mat5_empty();
    //
    //printf("Trans * Scale\n");
    //mat5_mul(trans, scale, result);
    //mat5_print(result);

    mat5 a = make_mat5(
        3.f, 2.f, 9.f, 6.f,
        9.f, 6.f, 3.f, 5.f,
        9.f, 7.f, 6.f, 5.f,
        1.f, 5.f, 3.f, 3.f
    );

    mat5 b = make_mat5(
        5.f, 1.f, 3.f, 2.f,
        1.f, 6.f, 4.f, 3.f,
        9.f, 2.f, 1.f, 4.f,
        7.f, 5.f, 5.f, 3.f
    );

    //--------------------------------------------------------------------------
    //// Test old and new translate
    //struct vec4 trans = (struct vec4) { 2.0f, 3.0f, 4.0f };
    //mat5 b = make_mat5_translate(trans);
    //mat5 result = make_mat5_empty();
    //
    //mat5_mul(a, b, result);
    //mat5_print(result);
    //
    //mat5_translate(a, trans);
    //mat5_print(a);
    //printf("translate valid: %i\n\n", mat5_equals(a, result));

    //--------------------------------------------------------------------------
    //// Test old and new scale
    //struct vec4 scale = (struct vec4) { 2.0f, 3.0f, 4.0f };
    //mat5 b = make_mat5_scale(scale);
    //mat5 result = make_mat5_empty();
    //
    //mat5_mul(a, b, result);
    //mat5_print(result);
    //
    //mat5_scale(a, scale);
    //mat5_print(a);
    //printf("scale valid: %i\n\n", mat5_equals(a, result));

    //--------------------------------------------------------------------------
    //// Test rot x
    //mat5 b = make_mat5_rotx(18);
    //mat5 result = make_mat5_empty();
    //
    //mat5_mul(a, b, result);
    //mat5_print(result);
    //
    //mat5_rotx(a, 18);
    //mat5_print(a);
    //printf("rot x valid: %i\n\n", mat5_equals(a, result));

    //--------------------------------------------------------------------------
    //// Test rot y
    //mat5 b = make_mat5_roty(18);
    //mat5 result = make_mat5_empty();

    //mat5_mul(a, b, result);
    //mat5_print(result);

    //mat5_roty(a, 18);
    //mat5_print(a);
    //printf("rot y valid: %i\n\n", mat5_equals(a, result));

    //--------------------------------------------------------------------------
    //// Test rot z
    //mat5 b = make_mat5_rotz(18);
    //mat5 result = make_mat5_empty();

    //mat5_mul(a, b, result);
    //mat5_print(result);

    //mat5_rotz(a, 18);
    //mat5_print(a);
    //printf("rot z valid: %i\n\n", mat5_equals(a, result));

    ////////////////////////////////////////////////////////////////////////////
}