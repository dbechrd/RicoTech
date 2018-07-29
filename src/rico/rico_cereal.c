enum ric_version
{
    V_EPOCH = 1,

    V_NEXT
};
#define V_CURRENT (V_NEXT - 1)
#define V_MAX 2147483647

struct ric_arena
{
    u32 size;
    u32 offset;
    void *buffer;
};

struct ric_stream
{
    s32 version;
    s32 v_max;   // Used to propagate an ARRAY_REMOVE's v_remove
    u8 mode;
    FILE *fp;
    struct ric_arena *arena;
};

struct ric_header
{
    u32 arena_size;
};

//------------------------------------------------------------------------------
// TODO: Remove test structs
struct ric_bar
{
    u32 ignore1;
    u32 check1;
    struct vec3 pos;
    u32 check2;
    u32 ignore2;
};

struct ric_foo
{
    u32 ignore1;
    u32 check1;
    struct ric_bar *bar;
    u32 *baz;
    u32 check2;
    u32 ignore2;
};

struct RICO_scene
{
    u32 object_count;
    struct RICO_object *objects;
    struct ric_foo foo;
};

struct ric_scene
{
    struct ric_header header;
    struct RICO_scene *scene;
};
//------------------------------------------------------------------------------

static size_t ric_fwrite(void const* buf, size_t size,
                         struct ric_stream *stream);
static size_t ric_fread(void *buf, size_t size, struct ric_stream *stream);
//static void rem_field(struct ric_stream *stream, enum ric_version v_add,
//                      enum ric_version v_remove, u32 size, void *field);
//static void rem_field_ptr(struct ric_stream *stream, enum ric_version v_add,
//                          enum ric_version v_remove, u32 size, void **field);
static void ric_uid(struct ric_stream *stream, struct uid *data);
static void ric_RICO_transform(struct ric_stream *stream,
                               struct RICO_transform *data);
static void ric_RICO_object(struct ric_stream *stream,
                            struct RICO_object *data);
static void ric_RICO_scene(struct ric_stream *stream, struct RICO_scene *data);

////////////////////////////////////////////////////////////////////////////////

#define RIC_READ  0
#define RIC_WRITE 1

#define V_CHECK_REMOVE(v_add, v_remove) \
    if (stream->version >= v_add && stream->version < v_remove) {
#define V_CHECK_ADD(v_add) \
    if (stream->version >= v_add) {
#define V_CHECK_END }

#define STREAM_RW(buf, size) \
    if (stream->mode == RIC_WRITE) { \
        ric_fwrite(buf, size, stream); \
    } else { \
        ric_fread(buf, size, stream); \
    }

#define ADD_FIELD(v_add, type, field) \
    V_CHECK_ADD(v_add) \
        STREAM_RW(&data->field, sizeof(data->field)); \
    V_CHECK_END

#define REM_FIELD(v_add, v_remove, type, field) \
    type field; \
    V_CHECK_REMOVE(v_add, v_remove) \
        STREAM_RW(&field, sizeof(field)); \
    V_CHECK_END

#define ADD_STRUCT(v_add, type, field, cereal) \
    V_CHECK_ADD(v_add) \
        cereal(stream, &data->field); \
    V_CHECK_END

#define REM_STRUCT(v_add, v_remove, type, field, cereal) \
    type field; \
    V_CHECK_REMOVE(v_add, v_remove) \
        cereal(stream, &field); \
    V_CHECK_END

#define ARENA_PUSH(field, count, size) \
    if (!field && stream->mode == RIC_READ) { \
        field = ric_arena_push(stream->arena, size * count); \
    }

#define ADD_FIELD_PTR(v_add, type, field) \
    V_CHECK_ADD(v_add) \
        ARENA_PUSH(data->field, 1, sizeof(*data->field)) \
        STREAM_RW(data->field, sizeof(*data->field)); \
    V_CHECK_END

#define REM_FIELD_PTR(v_add, v_remove, type, field) \
    type field; \
    V_CHECK_REMOVE(v_add, v_remove) \
        ARENA_PUSH(field, 1, sizeof(*field)) \
        RICO_ASSERT(field); \
        STREAM_RW(field, sizeof(*field)); \
    V_CHECK_END

#define ADD_STRUCT_PTR(v_add, type, field, cereal) \
    V_CHECK_ADD(v_add) \
        ARENA_PUSH(data->field, 1, sizeof(*data->field)) \
        cereal(stream, data->field); \
    V_CHECK_END

#define REM_STRUCT_PTR(v_add, v_remove, type, field, cereal) \
    type field; \
    V_CHECK_REMOVE(v_add, v_remove) \
        ARENA_PUSH(field, 1, sizeof(*field)) \
        cereal(stream, field); \
    V_CHECK_END

#define ADD_STRUCT_ARRAY(v_add, type, field, count, cereal) \
    V_CHECK_ADD(v_add) \
        ARENA_PUSH(data->field, count, sizeof(data->field[0])); \
        RICO_ASSERT(data->field); \
        for (u32 i = 0; i < count; ++i) { \
            cereal(stream, &data->field[i]); \
        } \
    V_CHECK_END

#define REM_STRUCT_ARRAY(v_add, v_remove, type, count, field) \
    type *field; \
    V_CHECK_REMOVE(v_add, v_remove) \
        ARENA_PUSH(field, count, sizeof(field[0])); \
        RICO_ASSERT(field); \
        for (u32 i = 0; i < count; ++i) { \
            cereal(stream, &field[i]); \
        } \
    V_CHECK_END

static inline void ric_arena_alloc(struct ric_arena *arena, u32 bytes)
{
    RICO_ASSERT(bytes);
    arena->size = bytes;
    arena->buffer = calloc(1, bytes);
}

static inline void ric_arena_free(struct ric_arena *arena)
{
    arena->size = 0;
    arena->offset = 0;
    free(arena->buffer);
}

static inline void ric_arena_clear(struct ric_arena *arena)
{
    arena->offset = 0;
    memset(arena->buffer, 0, arena->size);
}

static inline void ric_arena_copy(struct ric_arena *dst, struct ric_arena *src)
{
    ric_arena_alloc(dst, src->size);
    RICO_ASSERT(dst->size == src->size);
    dst->offset = src->offset;
    memcpy(dst->buffer, src->buffer, dst->size);
    RICO_ASSERT(!memcmp(dst->buffer, src->buffer, dst->size));
}

static inline void *ric_arena_push(struct ric_arena *arena, u32 bytes)
{
    RICO_ASSERT(arena->offset + bytes <= arena->size);
    void *ptr = (u8 *)arena->buffer + arena->offset;
    arena->offset += bytes;
    return ptr;
}

////////////////////////////////////////////////////////////////////////////////

static size_t ric_fwrite(void const* buf, size_t size,
                         struct ric_stream *stream)
{
    size_t bytes = fwrite(buf, 1, size, stream->fp);
    RICO_ASSERT(bytes == size);
    for (u32 i = 0; i < bytes; ++i)
    {
        u8 byte = *((const u8 *)buf + i);
        printf(" %02x", byte);
    }
    printf("\n");
    return bytes;
}

static size_t ric_fread(void *buf, size_t size, struct ric_stream *stream)
{
    size_t bytes = fread(buf, 1, size, stream->fp);
    RICO_ASSERT(bytes == size);
    for (u32 i = 0; i < bytes; ++i)
    {
        u8 byte = *((const u8 *)buf + i);
        printf(" %02x", byte);
    }
    printf("\n");
    return bytes;
}

static void ric_uid(struct ric_stream *stream, struct uid *data)
{
    ADD_FIELD(V_EPOCH, pkid, pkid);
    ADD_FIELD(V_EPOCH, enum RICO_hnd_type, type);
    ADD_FIELD(V_EPOCH, buf32, name);
}

static void ric_RICO_transform(struct ric_stream *stream,
                               struct RICO_transform *data)
{
    ADD_FIELD(V_EPOCH, struct vec3, position);
    ADD_FIELD(V_EPOCH, struct quat, orientation);
    ADD_FIELD(V_EPOCH, struct vec3, scale);
}

static void ric_RICO_object(struct ric_stream *stream,
                            struct RICO_object *data)
{
    ADD_STRUCT(V_EPOCH, struct uid, uid, ric_uid);
    ADD_FIELD(V_EPOCH, enum RICO_object_type, type);
    ADD_STRUCT(V_EPOCH, struct RICO_transform, xform, ric_RICO_transform);
    ADD_FIELD(V_EPOCH, pkid, mesh_id);
    ADD_FIELD(V_EPOCH, pkid, material_id);
}

static void ric_bar(struct ric_stream *stream, struct ric_bar *data)
{
    ADD_FIELD(V_EPOCH, u32, check1);
    ADD_FIELD(V_EPOCH, struct vec3, pos);
    ADD_FIELD(V_EPOCH, u32, check2);
}

static void ric_foo(struct ric_stream *stream, struct ric_foo *data)
{
    ADD_FIELD(V_EPOCH, u32, check1);
    ADD_STRUCT_PTR(V_EPOCH, struct ric_bar *, bar, ric_bar);
    ADD_FIELD_PTR(V_EPOCH, u32 *, baz);
    ADD_FIELD(V_EPOCH, u32, check2);
}

static void ric_RICO_scene(struct ric_stream *stream, struct RICO_scene *data)
{
    ADD_FIELD(V_EPOCH, u32, object_count);
    ADD_STRUCT_ARRAY(V_EPOCH, struct RICO_object, objects, data->object_count,
                     ric_RICO_object);
    ADD_STRUCT(V_EPOCH, struct ric_foo, foo, ric_foo);
}

static void ric_header(struct ric_stream *stream, struct ric_header *data)
{
    STREAM_RW(&stream->version, sizeof(stream->version));
    ADD_FIELD(V_EPOCH, u32, arena_size);
    if (!stream->arena->buffer)
    {
        ric_arena_alloc(stream->arena, data->arena_size);
    }
}

static void ric_scene(struct ric_stream *stream, struct ric_scene *data)
{
    ric_header(stream, (struct ric_header *)data);
    ADD_STRUCT_PTR(V_EPOCH, struct RICO_scene *, scene, ric_RICO_scene);
}

static void ric_close(struct ric_stream *stream)
{
    fclose(stream->fp);
}

#define TEST_OBJECTS 10
#define IGNORE_ME 0x4f434952

static void ric_test_write(struct ric_arena *arena, const char *filename)
{
    struct ric_stream stream_ = { 0 };
    struct ric_stream *stream = &stream_;

    // Allocate memory arena
    stream->arena = arena;
    stream->version = V_CURRENT;
    stream->mode = RIC_WRITE;
    stream->fp = fopen(filename, stream->mode ? "wb" : "rb");
    RICO_ASSERT(stream->fp);

    // Generate data
    struct ric_scene file = { 0 };
    file.header.arena_size = stream->arena->size;
    file.scene = ric_arena_push(stream->arena, sizeof(*file.scene));

    struct RICO_scene *scene = file.scene;
    scene->object_count = 10;
    scene->objects = ric_arena_push(stream->arena, sizeof(scene->objects[0]) *
                                    scene->object_count);
    for (u32 i = 0; i < TEST_OBJECTS; ++i)
    {
        scene->objects[i].xform.position = VEC3((float)i, 10.0f, 10.0f);
        scene->objects[i].xform.orientation = QUAT_IDENT;
        scene->objects[i].xform.scale = VEC3_ONE;
        scene->objects[i].material_id = i + 1;
        scene->objects[i].mesh_id = i + 1;
    }
    scene->foo.ignore1 = IGNORE_ME;
    scene->foo.check1 = 42;
    scene->foo.bar = ric_arena_push(stream->arena, sizeof(*scene->foo.bar));
    scene->foo.baz = ric_arena_push(stream->arena, sizeof(*scene->foo.baz));
    scene->foo.check2 = 43;
    scene->foo.ignore2 = IGNORE_ME;
    scene->foo.bar->ignore1 = IGNORE_ME;
    scene->foo.bar->check1 = 52;
    scene->foo.bar->pos = VEC3(1.0f, 2.0f, 3.0f);
    scene->foo.bar->check2 = 53;
    scene->foo.bar->ignore2 = IGNORE_ME;
    *scene->foo.baz = 1000;

    // Write data to file
    printf("[WRITE]\n");
    ric_scene(stream, &file);
    ric_close(stream);
    fflush(stdout);
}

static void ric_test_read(struct ric_arena *arena, const char *filename)
{
    struct ric_stream stream_ = { 0 };
    struct ric_stream *stream = &stream_;
    stream->arena = arena;
    stream->mode = RIC_READ;
    stream->fp = fopen(filename, stream->mode ? "wb" : "rb");
    RICO_ASSERT(stream->fp);

    struct ric_scene file = { 0 };
    printf("[READ]\n");
    ric_scene(stream, &file);

    struct RICO_scene *scene = file.scene;
    RICO_ASSERT(scene->object_count == 10);
    for (u32 i = 0; i < scene->object_count; ++i)
    {
        RICO_ASSERT(v3_equals(&scene->objects[i].xform.position,
                              &VEC3((float)i, 10.0f, 10.0f)));
        RICO_ASSERT(quat_equals(&scene->objects[i].xform.orientation,
                                &QUAT_IDENT));
        RICO_ASSERT(v3_equals(&scene->objects[i].xform.scale, &VEC3_ONE));
        RICO_ASSERT(scene->objects[i].obb.c.x == 0.0f);
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

    ric_close(stream);
    fflush(stdout);
}

static void ric_test_current()
{
    const u32 arena_size = KB(8);

    // NOTE: The reason I'm clearing and reusing the write arena is to ensure
    //       pointer comparisons don't create false positives during the data
    //       corruption check.

    // Write file into arena
    struct ric_arena arena = { 0 };
    ric_arena_alloc(&arena, arena_size);
    ric_test_write(&arena, "ric_test.bin");

    // Save copy and clear
    struct ric_arena arena_copy = { 0 };
    ric_arena_copy(&arena_copy, &arena);
    ric_arena_clear(&arena);

    // Read file into arena
    ric_test_read(&arena, "ric_1.bin");

    // Validate that data read is exactly the same as data written (aside from
    // IGNORE_ME values, which are intentionally not serialized to the file)
    for (u32 i = 0; i < arena.size; ++i)
    {
        if (((u8 *)arena.buffer)[i] != ((u8 *)arena_copy.buffer)[i])
        {
            u8 *byte = (u8 *)arena_copy.buffer + i;
            u32 *word = (u32 *)byte;
            if (*word == IGNORE_ME)
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

static void ric_test()
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
    ric_test_current();
    RICO_ASSERT(1);
}