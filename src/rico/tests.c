static u32 flp2(u32 x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x - (x >> 1);
}

static u32 pow2(u32 x)
{
    return (x & (x - 1)) == 0;
}

static void dlb_heap_print(dlb_heap *heap)
{
    printf("--- HEAP -----------\n");
    size_t size = dlb_heap_size(heap);
    s32 spaces = (s32)flp2(size);
    for (size_t i = 1; i <= size; i++)
    {
        if (pow2(i) && i > 1)
        {
            printf("\n");
            spaces /= 2;
        }
        for (u32 s = spaces - 1; s > 0; s--)
        {
            printf(" ");
        }
        printf("%d ", heap->nodes[i].priority);
        for (u32 s = spaces - 1; s > 0; s--)
        {
            printf(" ");
        }
    }
    printf("\n--- END ------------\n");
}

static void dlb_heap_test()
{
    dlb_heap _heap = { 0 };
    dlb_heap *heap = &_heap;
    void *peek, *pop;
    dlb_heap_init(heap);

    peek = dlb_heap_peek(heap);
    DLB_ASSERT(peek == NULL);

    dlb_heap_push(heap, 25, (void *)1);
    DLB_ASSERT(!dlb_heap_empty(heap));
    DLB_ASSERT(dlb_heap_size(heap) == 1);

    peek = dlb_heap_peek(heap);
    DLB_ASSERT((int)peek == 1);

    pop = dlb_heap_pop(heap);
    DLB_ASSERT((int)pop == 1);
    DLB_ASSERT(dlb_heap_empty(heap));
    DLB_ASSERT(dlb_heap_size(heap) == 0);

    int max = 100;
    for (int i = 1; i < max; i++)
    {
        dlb_heap_push(heap, i, (void *)i);
        dlb_heap_print(heap);
    }
    for (int i = 1; i < max; i++)
    {
        int data = (int)dlb_heap_pop(heap);
        int expected = max - i;
        DLB_ASSERT(data == expected);
        dlb_heap_print(heap);
    }
    DLB_ASSERT(dlb_heap_pop(heap) == NULL);

    dlb_heap_free(heap);
}

static void test_math()
{
    //new THREE.PerspectiveCamera(90, 1, 0.01, 1000).projectionMatrix
    struct mat4 proj_shadow_three = mat4_init(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1.0000200271606445, -1,
        0, 0, -0.02000020071864128, 0
    );
    mat4_transpose(&proj_shadow_three);

    struct mat4 proj_shadow_rico =
        mat4_init_perspective(1.0f, 0.01f, 1000.0f, 90.0f);

    UNUSED(proj_shadow_three);
    UNUSED(proj_shadow_rico);
    //RICO_ASSERT(mat4_equals(&proj_shadow_three, &proj_shadow_rico));

    //new THREE.PerspectiveCamera(54, 1600/900, 0.01, 1000).projectionMatrix
    struct mat4 proj_camera_three = mat4_init(
        1.1039683818817139, 0, 0, 0,
        0, 1.9626104831695557, 0, 0,
        0, 0, -1.0000200271606445, -1,
        0, 0, -0.02000020071864128, 0
    );
    mat4_transpose(&proj_camera_three);

    struct mat4 proj_camera_rico =
        mat4_init_perspective(1600.0f/900.0f, 0.01f, 1000.0f, 54.0f);

    UNUSED(proj_camera_three);
    UNUSED(proj_camera_rico);
    //RICO_ASSERT(mat4_equals(&proj_camera_three, &proj_camera_rico));

    //new THREE.OrthographicCamera(-1.0, 1.0, 1.0, -1.0, 0.01, 1000.0).projectionMatrix;
    struct mat4 ortho_camera_three = mat4_init(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -0.002000019885599613, 0,
        0, 0, -1.0000200271606445, 1
    );
    mat4_transpose(&ortho_camera_three);

    struct mat4 ortho_camera_rico =
        mat4_init_ortho(-1.0f, 1.0f, 1.0f, -1.0f, 0.01f, 1000.0f);

    RICO_ASSERT(mat4_equals(&ortho_camera_three, &ortho_camera_rico));

}

////////////////////////////////////////////////////////////////////////////////

static void test_geom()
{
    //TODO: Use Unity test framework (http://www.throwtheswitch.org/unity)
    //http://www.drdobbs.com/testing/unit-testing-in-c-tools-and-conventions/240156344

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

////////////////////////////////////////////////////////////////////////////////

static void test_hashtable()
{
    struct dlb_hash table_;
    struct dlb_hash *table = &table_;
    hashtable_init(table, "test table 1", 1, 8);

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
    hashtable_insert(table, key_id + 1, &data);
    hashtable_insert(table, key_id + 2, &data);
    hashtable_insert(table, key_id + 3, &data);

    int *lookup_uid = hashtable_search(table, key_id);
    RICO_ASSERT(*lookup_uid == data);
    int *lookup_uid1 = hashtable_search(table, key_id + 1);
    RICO_ASSERT(*lookup_uid1 == data);
    int *lookup_uid2 = hashtable_search(table, key_id + 2);
    RICO_ASSERT(*lookup_uid2 == data);
    int *lookup_uid3 = hashtable_search(table, key_id + 3);
    RICO_ASSERT(*lookup_uid3 == data);

    RICO_ASSERT(hashtable_delete(table, key_id));
    RICO_ASSERT(hashtable_delete(table, key_id + 1));
    RICO_ASSERT(hashtable_delete(table, key_id + 2));
    RICO_ASSERT(hashtable_delete(table, key_id + 3));
}

////////////////////////////////////////////////////////////////////////////////

static void test_ndc_macros()
{
    const struct vec2 top_left = VEC2(
        SCREEN_X(0),
        SCREEN_Y(0)
    );
    const struct vec2 bottom_right = VEC2(
        SCREEN_X(SCREEN_WIDTH),
        SCREEN_Y(SCREEN_HEIGHT)
    );
    const struct vec2 center = VEC2(
        SCREEN_X(SCREEN_WIDTH / 2),
        SCREEN_Y(SCREEN_HEIGHT / 2)
    );
    const struct vec2 top_left_neg = VEC2(
        SCREEN_X(-(r32)SCREEN_WIDTH),
        SCREEN_Y(-(r32)SCREEN_HEIGHT)
    );
    const struct vec2 center_neg = VEC2(
        SCREEN_X(-(r32)SCREEN_WIDTH / 2),
        SCREEN_Y(-(r32)SCREEN_HEIGHT / 2)
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

////////////////////////////////////////////////////////////////////////////////

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
    if (!mem_block) RICO_ERROR(RIC_ERR_BAD_ALLOC, "Failed to alloc for test pool");

    struct rico_pool *pool = mem_block;
    err = pool_init(pool, "Test pool", block_count, block_size);
    if (err) goto cleanup;

    struct pool_id id1, id2;
    struct rico_string *str;

    err = pool_add((struct hnd **)&str, pool);
    if (err) goto cleanup;
    id1 = str->hnd.id;

    hnd_init(&str->hnd, RIC_ASSET_OBJECT, "Test 1");

    err = pool_add((struct hnd **)&str, pool);
    if (err) goto cleanup;
    id2 = str->hnd.id;

    hnd_init(&str->hnd, RIC_ASSET_OBJECT, "Test 2");

    err = pool_remove(pool, id1);
    if (err) goto cleanup;
    err = pool_remove(pool, id2);
    if (err) goto cleanup;

cleanup:
    free(mem_block);
    return err;
}
#endif

////////////////////////////////////////////////////////////////////////////////

struct test_sav_object
{
    struct ric_uid uid;
    struct ric_transform xform;
    pkid mesh_id;
    pkid material_id;
};

struct test_sav_bar
{
    u32 ignore1;
    u32 check1;
    struct vec3 pos;
    u32 check2;
    u32 ignore2;
};

struct test_sav_foo
{
    u32 ignore1;
    u32 check1;
    struct test_sav_bar *bar;
    u32 *baz;
    u32 check2;
    u32 ignore2;
};

struct test_sav_scene
{
    u32 object_count;
    struct test_sav_object *objects;
    struct test_sav_foo foo;
};

struct test_sav_save
{
    struct test_sav_scene *scene;
};

static void test_sav_object_sav(struct ric_stream *stream,
                                struct test_sav_object *data)
{
    ADD_STRUCT(V_EPOCH, ric_uid, uid);
    ADD_STRUCT(V_EPOCH, ric_transform, xform);
    ADD_FIELD(V_EPOCH, pkid, mesh_id);
    ADD_FIELD(V_EPOCH, pkid, material_id);
}

static void test_sav_bar_sav(struct ric_stream *stream,
                             struct test_sav_bar *data)
{
    ADD_FIELD(V_EPOCH, u32, check1);
    ADD_FIELD(V_EPOCH, struct vec3, pos);
    ADD_FIELD(V_EPOCH, u32, check2);
}

static void test_sav_foo_sav(struct ric_stream *stream,
                             struct test_sav_foo *data)
{
    ADD_FIELD(V_EPOCH, u32, check1);
    ADD_STRUCT_PTR(V_EPOCH, test_sav_bar, bar);
    ADD_FIELD_PTR(V_EPOCH, u32 *, baz);
    ADD_FIELD(V_EPOCH, u32, check2);
}

static void test_sav_scene_sav(struct ric_stream *stream,
                               struct test_sav_scene *data)
{
    ADD_FIELD(V_EPOCH, u32, object_count);
    ADD_STRUCT_ARRAY(V_EPOCH, test_sav_object, objects, data->object_count);
    ADD_STRUCT(V_EPOCH, test_sav_foo, foo);
}

static void test_sav_save_sav(struct ric_stream *stream,
                              struct test_sav_save *data)
{
    ADD_STRUCT_PTR(V_EPOCH, test_sav_scene, scene);
}

static const u32 TEST_SAV_OBJECTS = 10;
static const u32 TEST_SAV_IGNORE_ME = 0x4f434952;

static void test_sav_write(struct ric_arena *arena, const char *filename)
{
    struct ric_stream stream_ = { 0 };
    struct ric_stream *stream = &stream_;
    stream->mode = RIC_SAV_WRITE;
    stream->filename = filename;
    stream->version = V_CURRENT;
    stream->arena = arena;

    ric_stream_open(stream);
    RICO_ASSERT(stream->fp);
    RICO_ASSERT(stream->arena->buffer);

    // Generate data
    struct test_sav_save file = { 0 };
    file.scene = ric_arena_push(stream->arena, sizeof(*file.scene));

    struct test_sav_scene *scene = file.scene;
    scene->object_count = TEST_SAV_OBJECTS;
    scene->objects = ric_arena_push(stream->arena, sizeof(scene->objects[0]) *
                                    scene->object_count);
    for (u32 i = 0; i < TEST_SAV_OBJECTS; ++i)
    {
        scene->objects[i].xform.position = VEC3((float)i, 10.0f, 10.0f);
        scene->objects[i].xform.orientation = QUAT_IDENT;
        scene->objects[i].xform.scale = VEC3_ONE;
        scene->objects[i].material_id = i + 1;
        scene->objects[i].mesh_id = i + 1;
    }
    scene->foo.ignore1 = TEST_SAV_IGNORE_ME;
    scene->foo.check1 = 42;
    scene->foo.bar = ric_arena_push(stream->arena, sizeof(*scene->foo.bar));
    scene->foo.baz = ric_arena_push(stream->arena, sizeof(*scene->foo.baz));
    scene->foo.check2 = 43;
    scene->foo.ignore2 = TEST_SAV_IGNORE_ME;
    scene->foo.bar->ignore1 = TEST_SAV_IGNORE_ME;
    scene->foo.bar->check1 = 52;
    scene->foo.bar->pos = VEC3(1.0f, 2.0f, 3.0f);
    scene->foo.bar->check2 = 53;
    scene->foo.bar->ignore2 = TEST_SAV_IGNORE_ME;
    *scene->foo.baz = 1000;

    // Write data to file
    printf("[WRITE]\n");
    test_sav_save_sav(stream, &file);
    ric_stream_close(stream);
    fflush(stdout);
}

static void test_sav_read(struct ric_arena *arena, const char *filename)
{
    struct ric_stream stream_ = { 0 };
    struct ric_stream *stream = &stream_;
    stream->arena = arena;
    stream->filename = filename;
    stream->mode = RIC_SAV_READ;

    ric_stream_open(stream);
    RICO_ASSERT(stream->fp);
    RICO_ASSERT(stream->arena->buffer);

    struct test_sav_save file = { 0 };
    printf("[READ]\n");
    test_sav_save_sav(stream, &file);

    struct test_sav_scene *scene = file.scene;
    RICO_ASSERT(scene->object_count == 10);
    for (u32 i = 0; i < scene->object_count; ++i)
    {
        RICO_ASSERT(v3_equals(&scene->objects[i].xform.position,
                              &VEC3((float)i, 10.0f, 10.0f)));
        RICO_ASSERT(quat_equals(&scene->objects[i].xform.orientation,
                                &QUAT_IDENT));
        RICO_ASSERT(v3_equals(&scene->objects[i].xform.scale, &VEC3_ONE));
        RICO_ASSERT(scene->objects[i].material_id == i + 1);
        RICO_ASSERT(scene->objects[i].mesh_id == i + 1);
    }

    RICO_ASSERT(scene->foo.ignore1 == 0);
    RICO_ASSERT(scene->foo.check1 == 42);
    RICO_ASSERT(scene->foo.bar);
    RICO_ASSERT(scene->foo.check2 == 43);
    RICO_ASSERT(scene->foo.ignore2 == 0);
    RICO_ASSERT(scene->foo.bar->ignore1 == 0);
    RICO_ASSERT(scene->foo.bar->check1 == 52);
    RICO_ASSERT(v3_equals(&scene->foo.bar->pos, &VEC3(1.0f, 2.0f, 3.0f)));
    RICO_ASSERT(scene->foo.bar->check2 == 53);
    RICO_ASSERT(scene->foo.bar->ignore2 == 0);
    RICO_ASSERT(scene->foo.baz);
    RICO_ASSERT(*scene->foo.baz == 1000);

    ric_stream_close(stream);
    fflush(stdout);
}

static void test_sav_current()
{
    const u32 arena_size = KB(8);

    // NOTE: The reason I'm clearing and reusing the write arena is to ensure
    //       pointer comparisons don't create false positives during the data
    //       corruption check.

    // Write file into arena
    struct ric_arena arena = { 0 };
    ric_arena_alloc(&arena, arena_size);
    test_sav_write(&arena, "test_sav.bin");

    // Save copy and clear
    struct ric_arena arena_copy = { 0 };
    ric_arena_copy(&arena_copy, &arena);
    ric_arena_clear(&arena);

    // Read file into arena
    test_sav_read(&arena, "test_sav.bin");

    // Validate that data read is exactly the same as data written (aside from
    // TEST_SAV_IGNORE_ME values, which are intentionally not serialized to the
    // file)
    for (u32 i = 0; i < arena.size; ++i)
    {
        if (((u8 *)arena.buffer)[i] != ((u8 *)arena_copy.buffer)[i])
        {
            u8 *byte = (u8 *)arena_copy.buffer + i;
            u32 *word = (u32 *)byte;
            if (*word == TEST_SAV_IGNORE_ME)
            {
                i += sizeof(u32) - sizeof(u8); // Magic ignore value, skip u32
            }
            else
            {
                RICO_ASSERT(0); // Data doesn't match, something went wrong!
            };
        }
    }

    ric_arena_free(&arena);
    ric_arena_free(&arena_copy);
}

static void test_sav()
{
    // Ensure atomic type sizes don't change
    RICO_ASSERT(sizeof(s32) == 4);
    RICO_ASSERT(sizeof(enum ric_version) == sizeof(s32));
    RICO_ASSERT(sizeof(u8) == 1);
    RICO_ASSERT(sizeof(u32) == 4);
    RICO_ASSERT(sizeof(r32) == 4);
    RICO_ASSERT(sizeof(float) == 4);
    RICO_ASSERT(sizeof(bool) == 1);
    RICO_ASSERT(sizeof(struct vec3) == 12);
    RICO_ASSERT(sizeof(struct quat) == 16);
    RICO_ASSERT(sizeof(buf32) == 32);

    //ric_test_1();
    test_sav_current();
    RICO_ASSERT(1);
}

////////////////////////////////////////////////////////////////////////////////

static void run_tests()
{
    dlb_heap_test();
    test_math();
    test_geom();
    test_hashtable();
    test_ndc_macros();
    //test_pool();
    test_sav();
}