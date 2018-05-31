static void test_geom()
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
static void test_hashtable()
{
    struct hash_table *table = &global_textures;

    int data = 123;

    const char key_str[] = "blah blah";
    u32 key_hash = hash_string(sizeof(key_str), key_str);
    hashtable_insert(table, key_hash, &data);

    int *lookup_str = hashtable_search(table, key_hash);
    RICO_ASSERT(*lookup_str == data);
    RICO_ASSERT(hashtable_delete(table, key_hash));

    //=================================================

    pkid key_id = 12345;
    hashtable_insert(table, key_id, &data);

    int *lookup_uid = hashtable_search(table, key_id);
    RICO_ASSERT(*lookup_uid == data);
    RICO_ASSERT(hashtable_delete(table, key_id));
}
static void test_ndc_macros()
{
    const struct vec2 top_left = VEC2(
        SCREEN_X(0.0f),
        SCREEN_Y(0.0f)
    );
    const struct vec2 bottom_right = VEC2(
        SCREEN_X(SCREEN_W),
        SCREEN_Y(SCREEN_H)
    );
    const struct vec2 center = VEC2(
        SCREEN_X(SCREEN_W / 2.0f),
        SCREEN_Y(SCREEN_H / 2.0f)
    );
    const struct vec2 top_left_neg = VEC2(
        SCREEN_X(-SCREEN_W),
        SCREEN_Y(-SCREEN_H)
    );
    const struct vec2 center_neg = VEC2(
        SCREEN_X(-SCREEN_W / 2.0f),
        SCREEN_Y(-SCREEN_H / 2.0f)
    );
    RICO_ASSERT(top_left.x == -1.0f);
    RICO_ASSERT(top_left.y == 1.0f);
    RICO_ASSERT(bottom_right.x == 1.0f);
    RICO_ASSERT(bottom_right.y == -1.0f);
    RICO_ASSERT(center.y == 0.0f);
    RICO_ASSERT(center.y == 0.0f);
    RICO_ASSERT(top_left_neg.x == -1.0f);
    RICO_ASSERT(top_left_neg.y == 1.0f);
    RICO_ASSERT(center_neg.y == 0.0f);
    RICO_ASSERT(center_neg.y == 0.0f);
}
#if 0
static int test_pool()
{
    enum rico_error err;

    u32 block_count = 8;
    u32 block_size = sizeof(struct rico_texture);
    u32 pool_size = POOL_SIZE(block_count, block_size);

    u32 pool_block_tags_sz = POOL_BLOCK_TAGS_SIZE(8);
    u32 pool_tags_sz = POOL_TAGS_SIZE(8);
    u32 pool_blocks_sz = POOL_BLOCKS_SIZE(8, block_size);
    u32 pool_sz = POOL_SIZE(block_count, block_size);

    RICO_ASSERT(pool_block_tags_sz == 32);
    RICO_ASSERT(pool_tags_sz == 8 * sizeof(struct pool_tag));
    RICO_ASSERT(pool_blocks_sz == 8 * block_size);
    RICO_ASSERT(pool_sz == sizeof(struct rico_pool) +
                32 +
                8 * sizeof(struct pool_tag) +
                8 * block_size);

    u32 pool_block_tags_offs = POOL_BLOCK_TAGS_OFFSET();
    u32 pool_tags_offs = POOL_TAGS_OFFSET(8);
    u32 pool_blocks_offs = POOL_BLOCKS_OFFSET(8);

    RICO_ASSERT(pool_block_tags_offs == sizeof(struct rico_pool));
    RICO_ASSERT(pool_tags_offs == sizeof(struct rico_pool) + 32);
    RICO_ASSERT(pool_blocks_offs == sizeof(struct rico_pool) + 32 +
                8 * sizeof(struct pool_tag));

    void *mem_block = calloc(1, pool_size);
    if (!mem_block) RICO_ERROR(ERR_BAD_ALLOC, "Failed to alloc for test pool");

    struct rico_pool *pool = mem_block;
    err = pool_init(pool, "Test pool", block_count, block_size);
    if (err) goto cleanup;

    struct pool_id id1, id2;
    struct rico_string *str;

    err = pool_add((struct hnd **)&str, pool);
    if (err) goto cleanup;
    id1 = str->hnd.id;

    hnd_init(&str->hnd, RICO_HND_OBJECT, "Test 1");

    err = pool_add((struct hnd **)&str, pool);
    if (err) goto cleanup;
    id2 = str->hnd.id;

    hnd_init(&str->hnd, RICO_HND_OBJECT, "Test 2");

    err = pool_remove(pool, id1);
    if (err) goto cleanup;
    err = pool_remove(pool, id2);
    if (err) goto cleanup;

cleanup:
    free(mem_block);
    return err;
}
#endif
static void run_tests()
{
    test_geom();
    test_hashtable();
    test_ndc_macros();
    //test_pool();
}