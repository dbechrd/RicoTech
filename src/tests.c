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
    struct hash_table *table = &global_textures;

    int data = 123;

    const char key_str[] = "blah blah";
    hashtable_insert_str(table, key_str, &data, sizeof(data));

    int *lookup_str = hashtable_search_str(table, key_str);
    RICO_ASSERT(*lookup_str == data);
    RICO_ASSERT(hashtable_delete_str(table, key_str));

    //=================================================

    uid key_uid = 12345;
    hashtable_insert_uid(table, key_uid, &data, sizeof(data));

    int *lookup_uid = hashtable_search_uid(table, key_uid);
    RICO_ASSERT(*lookup_uid == data);
    RICO_ASSERT(hashtable_delete_uid(table, key_uid));
}

int test_pool()
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
    if (err) return err;

    struct pool_id id1, id2;
    struct rico_texture *tex;

    printf("============================================================\n"
           " ADD ID 1\n"
           "============================================================\n");
    err = pool_add((struct hnd **)&tex, pool);
    if (err) return err;
    id1 = tex->hnd.id;

    hnd_init(&tex->hnd, RICO_HND_TEXTURE, "Test 1");
    tex->bpp = 16;
    tex->gl_id = 1;
    tex->gl_target = 1;
    tex->height = 64;
    tex->width = 64;

    printf("============================================================\n"
           " ADD ID 2\n"
           "============================================================\n");
    err = pool_add((struct hnd **)&tex, pool);
    if (err) return err;
    id2 = tex->hnd.id;

    hnd_init(&tex->hnd, RICO_HND_TEXTURE, "Test 2");
    tex->bpp = 32;
    tex->gl_id = 2;
    tex->gl_target = 2;
    tex->height = 128;
    tex->width = 128;

    printf("============================================================\n"
           " REMOVE ID 1\n"
           "============================================================\n");
    err = pool_remove(pool, id1);
    if (err) return err;
    printf("============================================================\n"
           " REMOVE ID 2\n"
           "============================================================\n");
    err = pool_remove(pool, id2);
    return err;
}

void run_tests()
{
    //test_geom();
    //test_hashtable();
    test_pool();
}