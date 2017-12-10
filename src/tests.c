void test_geom()
{
    //TODO: Use Unity test framework (http://www.throwtheswitch.org/unity)
    //http://www.drdobbs.com/testing/unit-testing-in-c-tools-and-conventions/240156344
    //run_tests();

    ////////////////////////////////////////////////////////////////////////////

    //// Test translate / scale order
    //mat4 scale = mat4_scale((struct vec3) { 10.0f, 11.0f, 12.0f });
    //mat4 trans = mat4_translate((struct vec3) { 2.f, 3.f, 4.f });
    //mat4 result = make_mat4_empty();
    //
    //printf("Trans * Scale\n");
    //mat4_mul(trans, scale, result);
    //mat4_print(result);

    struct mat4 a = mat4_init(
        3.f, 2.f, 9.f, 6.f,
        9.f, 6.f, 3.f, 5.f,
        9.f, 7.f, 6.f, 5.f,
        1.f, 5.f, 3.f, 3.f
    );

    struct mat4 b = mat4_init(
        5.f, 1.f, 3.f, 2.f,
        1.f, 6.f, 4.f, 3.f,
        9.f, 2.f, 1.f, 4.f,
        7.f, 5.f, 5.f, 3.f
    );

    mat4_print(&a);
    mat4_print(&b);

    mat4_mul(&a, &b);
    mat4_print(&a);
}

void test_hashtable()
{
    rico_hashtable_init();
    struct hash_table *table = &global_textures;

    int data1 = 123;
    int data2 = 456;
    int data3 = 789;

    const char key1[] = "blah blah";
    const char key2[] = "foo foo";
    const char key3[] = "this is really great!";

    hashtable_insert_str(table, key1, &data1);
    hashtable_insert_str(table, key2, &data2);
    hashtable_insert_str(table, key3, &data3);

    int *lookup1 = hashtable_search_str(table, key1);
    int *lookup2 = hashtable_search_str(table, key2);
    int *lookup3 = hashtable_search_str(table, key3);

    RICO_ASSERT(*lookup1 == data1);
    RICO_ASSERT(*lookup2 == data2);
    RICO_ASSERT(*lookup3 == data3);
}
