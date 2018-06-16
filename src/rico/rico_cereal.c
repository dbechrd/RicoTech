#define RIC_READ  0
#define RIC_WRITE 1

#define FIELD_RW(buf, size, count, fp)    \
    if (stream->mode == RIC_WRITE) {      \
        ric_fwrite(buf, size, count, fp); \
    } else {                              \
        ric_fread(buf, size, count, fp);  \
    }

#define ADD_FIELD(v_add, type, field) \
    if (stream->version >= v_add) {   \
        ric_##type(stream, (field));  \
    }

#define REMOVE_FIELD(v_add, v_remove, type, field)               \
    if (stream->version >= v_add && stream.version < v_remove) { \
        ric_##type(stream, (field));                             \
    }

#define ADD_ARRAY(v_add, type, count, field)                          \
    if (stream->version >= v_add) {                                   \
        if (!*(field) && count && stream->mode == RIC_READ) {         \
            *(field) = (void *)((u8 *)stream->data + stream->offset); \
        }                                                             \
        RICO_ASSERT(*(field));                                        \
        for (u32 i = 0; i < count; ++i) {                             \
            ric_##type(stream, &(*(field))[i]);                       \
        }                                                             \
    }

#define REMOVE_ARRAY(v_add, v_remove, type, count, field)        \
    if (stream->version >= v_add && stream.version < v_remove) { \
        RICO_ASSERT(field);                                      \
        for (u32 i = 0; i < count; ++i) {                        \
            ric_##type(stream, &(field)[i]);                     \
        }                                                        \
    }

#define ADD_BUFFER(v_add, buffer)                        \
    if (stream->version >= v_add) {                      \
        FIELD_RW(buffer, sizeof(buffer), 1, stream); \
    }

#define REMOVE_BUFFER(v_add, v_remove, buffer)                   \
    if (stream->version >= v_add && stream.version < v_remove) { \
        FIELD_RW(buffer, sizeof(buffer), 1, stream);         \
    }

//typedef void (*ric_cereal)(struct ric_stream *stream, void *data);

/*
struct uid
{
    pkid pkid;
    enum RICO_hnd_type type;
    char name[32];
};

struct RICO_transform
{
    struct vec3 position;
    struct quat orientation;
    struct vec3 scale;
    struct mat4 matrix;             // Serial: Calculate, don't serialize
    struct mat4 matrix_inverse;     // Serial: Calculate, don't serialize 
};

struct RICO_object
{
    struct uid uid;
    u32 type;
    struct RICO_transform xform;
    struct RICO_bbox bbox;          // Serial: Calculate, don't serialize
    struct RICO_bbox bbox_world;    // Serial: Calculate, don't serialize
    struct RICO_obb obb;            // Serial: Calculate, don't serialize
    struct sphere sphere;           // Serial: Calculate, don't serialize
    bool selected;                  // Serial: Set this shit to false, bro
    pkid mesh_id;
    pkid material_id;
};
*/

// TODO: Move this to its own file if it's useful
struct RICO_scene
{
    u32 object_count;
    struct RICO_object *objects;
};

static size_t ric_fwrite(void const* buf, size_t size, size_t count,
                         struct ric_stream *stream)
{
    stream->offset += size * count;
    return fwrite(buf, size, count, stream->fp);
}

static size_t ric_fread(void *buf, size_t size, size_t count,
                        struct ric_stream *stream)
{
    stream->offset += size * count;
    return fread(buf, size, count, stream->fp);
}

static void ric_s32(struct ric_stream *stream, s32 *data)
{
    RICO_ASSERT(sizeof(s32) == 4);
    FIELD_RW(data, sizeof(s32), 1, stream);
}

static void ric_u32(struct ric_stream *stream, u32 *data)
{
    RICO_ASSERT(sizeof(u32) == 4);
    FIELD_RW(data, sizeof(u32), 1, stream);
}

static void ric_float(struct ric_stream *stream, float *data)
{
    RICO_ASSERT(sizeof(float) == 4);
    FIELD_RW(data, sizeof(float), 1, stream);
}

static void ric_bool(struct ric_stream *stream, bool *data)
{
    RICO_ASSERT(sizeof(bool) == 1);
    FIELD_RW(data, sizeof(bool), 1, stream);
}

static void ric_enum(struct ric_stream *stream, void *data)
{
    ric_s32(stream, data);
}

static void ric_vec3(struct ric_stream *stream, struct vec3 *data)
{
    RICO_ASSERT(sizeof(struct vec3) == 12);
    FIELD_RW(data, sizeof(struct vec3), 1, stream);
}

static void ric_quat(struct ric_stream *stream, struct quat *data)
{
    RICO_ASSERT(sizeof(struct quat) == 16);
    FIELD_RW(data, sizeof(struct quat), 1, stream);
}

static void ric_uid(struct ric_stream *stream, struct uid *data)
{
    ADD_FIELD(V_EPOCH, u32, &data->pkid);
    ADD_FIELD(V_EPOCH, enum, &data->type);
    ADD_BUFFER(V_EPOCH, data->name);
}

static void ric_RICO_transform(struct ric_stream *stream,
                               struct RICO_transform *data)
{
    ADD_FIELD(V_EPOCH, vec3, &data->position);
    ADD_FIELD(V_EPOCH, quat, &data->orientation);
    ADD_FIELD(V_EPOCH, vec3, &data->scale);
}

static void ric_RICO_object(struct ric_stream *stream, struct RICO_object *data)
{
    ADD_FIELD(V_EPOCH, uid, &data->uid);
    ADD_FIELD(V_EPOCH, enum, &data->type);
    ADD_FIELD(V_EPOCH, RICO_transform, &data->xform);
    ADD_FIELD(V_EPOCH, u32, &data->mesh_id);
    ADD_FIELD(V_EPOCH, u32, &data->material_id);
}

static void ric_RICO_scene(struct ric_stream *stream, struct RICO_scene *data)
{
    ADD_FIELD(V_EPOCH, u32, &data->object_count);
    ADD_ARRAY(V_EPOCH, RICO_object, data->object_count, &data->objects);
}

static void ric_test_1()
{
    struct ric_stream stream = { 0 };
    stream.mode = false;
    stream.fp = fopen("ric_test_1.bin", stream.mode ? "wb" : "rb");
    RICO_ASSERT(stream.fp);

    ric_fread(&stream.version, sizeof(stream.version), 1, &stream);
    fseek(stream.fp, 0, SEEK_END);
    stream.length = ftell(stream.fp);
    rewind(stream.fp);

    struct RICO_scene *scene = calloc(1, stream.length);
    ric_RICO_scene(&stream, scene);
    fclose(stream.fp);

    for (u32 i = 0; i < scene->object_count; ++i)
    {
        RICO_ASSERT(scene->objects[i].material_id == 0);
        RICO_ASSERT(scene->objects[i].mesh_id == 0);
    }
}

static const u32 test_objects = 10;
static void ric_test_write()
{
    struct ric_stream stream = { 0 };
    struct RICO_scene *write = calloc(
        1,
        sizeof(struct RICO_scene) + sizeof(write->objects[0]) * test_objects
    );
    write->objects = (void *)((u8 *)write + sizeof(struct RICO_scene));
    write->object_count = 10;

    struct RICO_object *obj;
    for (u32 i = 0; i < test_objects; ++i)
    {
        obj = &write->objects[i];
        obj->xform.position = VEC3((float)i, 10.0f, 10.0f);
        obj->xform.orientation = QUAT_IDENT;
        obj->xform.scale = VEC3_ONE;
        obj->material_id = i + 1;
        obj->mesh_id = i + 1;
    }

    stream.version = V_CURRENT;
    stream.mode = RIC_WRITE;
    stream.fp = fopen("ric_test.bin", stream.mode ? "wb" : "rb");
    RICO_ASSERT(stream.fp);

    ric_fwrite(&stream.version, sizeof(stream.version), 1, &stream);
    ric_RICO_scene(&stream, write);
    fclose(stream.fp);
    free(write);
}

static void ric_test_read()
{
    struct ric_stream stream = { 0 };
    stream.mode = RIC_READ;
    stream.fp = fopen("ric_test.bin", stream.mode ? "wb" : "rb");
    RICO_ASSERT(stream.fp);

    fseek(stream.fp, 0, SEEK_END);
    stream.length = ftell(stream.fp);
    rewind(stream.fp);
    ric_fread(&stream.version, sizeof(stream.version), 1, &stream);

    //stream.data = calloc(1, stream.length - stream.offset);
    struct RICO_scene *read = calloc(
        1,
        sizeof(struct RICO_scene) + sizeof(read->objects[0]) * test_objects
    );
    stream.data = read;
    ric_RICO_scene(&stream, read);

    for (u32 i = 0; i < read->object_count; ++i)
    {
        RICO_ASSERT(v3_equals(&read->objects[i].xform.position, &VEC3((float)i, 10.0f, 10.0f)));
        RICO_ASSERT(quat_equals(&read->objects[i].xform.orientation, &QUAT_IDENT));
        RICO_ASSERT(v3_equals(&read->objects[i].xform.scale, &VEC3_ONE));
        RICO_ASSERT(read->objects[i].obb.c.x == 0.0f);
        RICO_ASSERT(read->objects[i].material_id == i + 1);
        RICO_ASSERT(read->objects[i].mesh_id == i + 1);
    }

    fclose(stream.fp);
    free(read);
}

static void ric_test_current()
{
    ric_test_write();
    ric_test_read();
}

static void ric_test()
{
    //ric_test_1();
    ric_test_current();
    RICO_ASSERT(1);
}