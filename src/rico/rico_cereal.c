#define RIC_READ  0
#define RIC_WRITE 1

static void ric_arena_alloc(struct ric_arena *arena, u32 bytes)
{
    arena->size = bytes;
    arena->buffer = calloc(1, bytes);
}

static void ric_arena_free(struct ric_arena *arena)
{
    arena->size = 0;
    arena->offset = 0;
    free(arena->buffer);
}

static void *ric_arena_push(struct ric_arena *arena, u32 bytes)
{
    RICO_ASSERT(arena->offset + bytes <= arena->size);
    void *ptr = (u8 *)arena->buffer + arena->offset;
    arena->offset += bytes;
    return ptr;
}

#define FIELD_RW(buf, size) \
    if (stream->mode == RIC_WRITE) { \
        ric_fwrite(buf, size, stream); \
    } else { \
        ric_fread(buf, size, stream); \
    }


//#define REM_FIELD(v_add, v_remove, field) \
//    if (stream->version >= v_add && stream->version < v_remove) { \
//        printf("<" #field ">"); \
//        FIELD_RW(&data->field, sizeof(data->field)); \
//        printf("</" #field ">\n"); \
//    }
//static void rem_field(struct ric_stream *stream, enum ric_version v_add,
//                      enum ric_version v_remove, u32 size, void **field)
//{
//    if (stream->version >= v_add && stream->version < v_remove) {
//        FIELD_RW(*field, size);
//    }
//}
//#define ADD_FIELD(v_add, field) REM_FIELD(v_add, V_MAX, field)

//#define REM_FIELD_PTR(v_add, v_remove, field) \
//    if (stream->version >= v_add && stream->version < v_remove) { \
//        if (!data->field && stream->mode == RIC_READ) { \
//            data->field = ric_arena_push(&stream->arena, \
//                                         sizeof(*data->field)); \
//        } \
//        RICO_ASSERT(data->field); \
//        printf("<" #field ">"); \
//        FIELD_RW(data->field, sizeof(*data->field)); \
//        printf("</" #field ">\n"); \
//    }
static void rem_field(struct ric_stream *stream, enum ric_version v_add,
                      enum ric_version v_remove, u32 size, void *field)
{
    if (stream->version >= v_add && stream->version < v_remove) {
        FIELD_RW(field, size);
    }
}
#define REM_FIELD(v_add, v_remove, type, field) \
    type field; \
    rem_field(stream, v_add, v_remove, sizeof(field), &field);
#define ADD_FIELD(v_add, type, field) \
    rem_field(stream, v_add, V_MAX, sizeof(data->field), &data->field);

static void rem_field_ptr(struct ric_stream *stream, enum ric_version v_add,
                          enum ric_version v_remove, u32 size, void **field)
{
    if (stream->version >= v_add && stream->version < v_remove) {
        if (!*field && stream->mode == RIC_READ) {
            *field = ric_arena_push(&stream->arena, size);
        }
        RICO_ASSERT(*field);
        FIELD_RW(*field, size);
    }
}
#define REM_FIELD_PTR(v_add, v_remove, type, field) \
    type field; \
    rem_field_ptr(stream, v_add, v_remove, sizeof(*field), &field);
#define ADD_FIELD_PTR(v_add, type, field) \
    rem_field_ptr(stream, v_add, V_MAX, sizeof(*data->field), &data->field);

#define REM_STRUCT(v_add, v_remove, type, field) \
    if (stream->version >= v_add && stream->version < v_remove) { \
        printf("<" #type ">\n"); \
        ric_##type(stream, &data->field); \
        printf("</" #type ">\n"); \
    }
#define ADD_STRUCT(v_add, type, field) REM_STRUCT(v_add, V_MAX, type, field)

#define REM_STRUCT_PTR(v_add, v_remove, type, field) \
    if (stream->version >= v_add && stream->version < v_remove) { \
        if (!data->field && stream->mode == RIC_READ) { \
            data->field = ric_arena_push(&stream->arena, \
                                         sizeof(*data->field)); \
        } \
        RICO_ASSERT(data->field); \
        printf("<" #type ">\n"); \
        ric_##type(stream, data->field); \
        printf("</" #type ">\n"); \
    }
#define ADD_STRUCT_PTR(v_add, type, field) \
    REM_STRUCT_PTR(v_add, V_MAX, type, field)

#define ADD_ARRAY(v_add, type, field, count) \
    if (stream->version >= v_add) { \
        if (!(data->field) && count && stream->mode == RIC_READ) { \
            data->field = ric_arena_push(&stream->arena, \
                                         sizeof(data->field[0]) * count); \
        } \
        RICO_ASSERT(data->field); \
        printf("<" #type " count=\"%u\">\n", count); \
        for (u32 i = 0; i < count; ++i) { \
            ric_##type(stream, &data->field[i]); \
        } \
        printf("</" #type ">\n"); \
    }

#define REM_ARRAY(v_add, v_remove, type, count, field) \
    if (stream->version >= v_add && stream->version < v_remove) { \
        RICO_ASSERT(field); \
        for (u32 i = 0; i < count; ++i) { \
            ric_##type(stream, &field[i]); \
        } \
    }

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

struct ric_header
{
    u32 arena_size;
};

struct ric_scene
{
    struct ric_header header;
    struct RICO_scene scene;
};

typedef struct vec3 vec3_t;
typedef struct quat quat_t;
typedef struct uid uid_t;
typedef struct RICO_transform RICO_transform_t;
typedef struct RICO_object RICO_object_t;
typedef struct ric_bar ric_bar_t;
typedef struct ric_foo ric_foo_t;
typedef struct RICO_scene RICO_scene_t;

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
    return bytes;
}

static void ric_uid_t(struct ric_stream *stream, struct uid *data)
{
    ADD_FIELD(V_EPOCH, pkid, pkid);
    ADD_FIELD(V_EPOCH, enum RICO_hnd_type, type);
    ADD_FIELD(V_EPOCH, buf32, name);
}

static void ric_RICO_transform_t(struct ric_stream *stream,
                                 struct RICO_transform *data)
{
    ADD_FIELD(V_EPOCH, struct vec3, position);
    ADD_FIELD(V_EPOCH, struct quat, orientation);
    ADD_FIELD(V_EPOCH, struct vec3, scale);
}

static void ric_RICO_object_t(struct ric_stream *stream,
                              struct RICO_object *data)
{
    ADD_STRUCT(V_EPOCH, uid_t, uid);
    ADD_FIELD(V_EPOCH, enum RICO_object_type, type);
    ADD_STRUCT(V_EPOCH, RICO_transform_t, xform);
    ADD_FIELD(V_EPOCH, pkid, mesh_id);
    ADD_FIELD(V_EPOCH, pkid, material_id);
}

static void ric_ric_bar_t(struct ric_stream *stream, struct ric_bar *data)
{
    ADD_FIELD(V_EPOCH, u32, check1);
    ADD_FIELD(V_EPOCH, struct vec3, pos);
    ADD_FIELD(V_EPOCH, u32, check2);
}

static void ric_ric_foo_t(struct ric_stream *stream, struct ric_foo *data)
{
    ADD_FIELD(V_EPOCH, u32, check1);
    ADD_STRUCT_PTR(V_EPOCH, ric_bar_t, bar);
    ADD_FIELD_PTR(V_EPOCH, u32 *, baz);
    ADD_FIELD(V_EPOCH, u32, check2);
}

static void ric_RICO_scene_t(struct ric_stream *stream, struct RICO_scene *data)
{
    ADD_FIELD(V_EPOCH, u32, object_count);
    ADD_ARRAY(V_EPOCH, RICO_object_t, objects, data->object_count);
    ADD_STRUCT(V_EPOCH, ric_foo_t, foo);
}

static void ric_header(struct ric_stream *stream, struct ric_header *data)
{
    FIELD_RW(&stream->version, sizeof(stream->version));
    ADD_FIELD(V_EPOCH, u32, arena_size);
    if (stream->mode == RIC_READ)
    {
        ric_arena_alloc(&stream->arena, data->arena_size);
    }
}

static void ric_scene(struct ric_stream *stream, struct ric_scene *data)
{
    ric_header(stream, (struct ric_header *)data);
    ADD_STRUCT(V_EPOCH, RICO_scene_t, scene);
}

static void ric_close(struct ric_stream *stream)
{
    fclose(stream->fp);
    ric_arena_free(&stream->arena);
}

static const u32 test_objects = 10;
static void ric_test_write()
{
    struct ric_stream stream_ = { 0 };
    struct ric_stream *stream = &stream_;
    
    // Allocate memory arena
    ric_arena_alloc(&stream->arena, KB(8));
    stream->version = V_CURRENT;
    stream->mode = RIC_WRITE;
    stream->fp = fopen("ric_test.bin", stream->mode ? "wb" : "rb");
    RICO_ASSERT(stream->fp);

    // Generate data
    struct ric_scene file = { 0 };
    file.header.arena_size = stream->arena.size;

    struct RICO_scene *scene = &file.scene;
    scene->object_count = 10;
    scene->objects = ric_arena_push(&stream->arena, sizeof(scene->objects[0]) *
                                    scene->object_count);
    for (u32 i = 0; i < test_objects; ++i)
    {
        scene->objects[i].xform.position = VEC3((float)i, 10.0f, 10.0f);
        scene->objects[i].xform.orientation = QUAT_IDENT;
        scene->objects[i].xform.scale = VEC3_ONE;
        scene->objects[i].material_id = i + 1;
        scene->objects[i].mesh_id = i + 1;
    }
    scene->foo.ignore1 = 99;
    scene->foo.check1 = 42;
    scene->foo.bar = ric_arena_push(&stream->arena, sizeof(*scene->foo.bar));
    scene->foo.baz = ric_arena_push(&stream->arena, sizeof(*scene->foo.baz));
    scene->foo.check2 = 43;
    scene->foo.ignore2 = 99;
    scene->foo.bar->ignore1 = 99;
    scene->foo.bar->check1 = 52;
    scene->foo.bar->pos = VEC3(1.0f, 2.0f, 3.0f);
    scene->foo.bar->check2 = 53;
    scene->foo.bar->ignore2 = 99;
    *scene->foo.baz = 1000;

    // Write data to file
    printf("[WRITE]\n");
    ric_scene(stream, &file);
    ric_close(stream);
    fflush(stdout);
}

static void ric_test_read()
{
    struct ric_stream stream_ = { 0 };
    struct ric_stream *stream = &stream_;
    stream->mode = RIC_READ;
    stream->fp = fopen("ric_test.bin", stream->mode ? "wb" : "rb");
    RICO_ASSERT(stream->fp);

    struct ric_scene file = { 0 };
    printf("[READ]\n");
    ric_scene(stream, &file);

    struct RICO_scene *scene = &file.scene;
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
    ric_test_write();
    ric_test_read();
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
    RICO_ASSERT(0);
}