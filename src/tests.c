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
    struct hnd blah1 = { TRANSIENT, 123 };
    struct hnd blah2 = { TRANSIENT, 456 };
    struct hnd blah3 = { TRANSIENT, 789 };

    const char bleh1[] = "blah blah";
    const char bleh2[] = "foo foo";
    const char bleh3[] = "this is really great!";

    hash_key key1 = hashgen_strlen(bleh1, sizeof(bleh1));
    hash_key key2 = hashgen_strlen(bleh2, sizeof(bleh2));
    hash_key key3 = hashgen_strlen(bleh3, sizeof(bleh3));

    struct hash_table *table = &global_textures;

    rico_hashtable_init();
    hashtable_insert(table, key1, blah1);
    hashtable_insert(table, key2, blah2);
    hashtable_insert(table, key3, blah3);

    struct hnd lookup1 = hashtable_search(table, key1);
    struct hnd lookup2 = hashtable_search(table, key2);
    struct hnd lookup3 = hashtable_search(table, key3);

    RICO_ASSERT(lookup1.value == blah1.value);
    RICO_ASSERT(lookup2.value == blah2.value);
    RICO_ASSERT(lookup3.value == blah3.value);
}