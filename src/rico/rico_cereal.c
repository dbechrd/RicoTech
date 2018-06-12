#define FIELD_RW(buf, size, count, fp) \
    if (config->write) { \
        ric_fwrite(buf, size, count, fp); \
    } else { \
        ric_fread(buf, size, count, fp); \
    }

//#define ADD_FIELD(v_add, type, field) \
//    if (config->version > v_add) { \
//        FIELD_RW(field, sizeof(type), 1, config->fp); \
//    }
//
//#define REMOVE_FIELD(v_add, v_remove, type, field) \
//    if (config->version >= v_add && config.version < v_remove) { \
//        FIELD_RW(field, sizeof(type), 1, config->fp); \
//    }

#define ADD_FIELD(v_add, type, field) \
    if (config->version >= v_add) { \
        cereal_##type(config, field); \
    }

#define REMOVE_FIELD(v_add, v_remove, type, field) \
    if (config->version >= v_add && config.version < v_remove) { \
        cereal_##type(config, &field); \
    }


enum ric_cereal_version
{
    RIC_V_EPOCH = 1,
    RIC_V_ADD_FLAG1,

    RIC_V_NEXT
};
#define RIC_V_CURRENT (RIC_V_NEXT - 1)

typedef void (*ric_cereal)(struct ric_config *config, void *data);

struct ric_config
{
    u32 version;
    bool write;
    FILE *fp;
};

struct ric_test_entity
{
    //struct uid uid;
    //struct vec3 pos;    // TODO: Add this
    //struct quat orient; // TODO: Add this, then remove this

    bool flag1;
    pkid tex_id;
};

struct ric_data
{
    struct ric_test_entity entities[10];
};

static size_t ric_fwrite(void const* buf, size_t size, size_t count, FILE *fp)
{
    return fwrite(buf, size, count, fp);
}

static size_t ric_fread(void *buf, size_t size, size_t count, FILE *fp)
{
    return fread(buf, size, count, fp);
}

static void cereal_data(struct ric_config *config, struct ric_data *data)
{
    for (u32 i = 0; i < ARRAY_COUNT(data->entities); ++i)
    {
        ADD_FIELD(RIC_V_EPOCH, ric_test_entity, &data->entities[i]);
    }
}

static void cereal_ric_test_entity(struct ric_config *config,
                                   struct ric_test_entity *data)
{
    ADD_FIELD(RIC_V_ADD_FLAG1, bool, &data->flag1);
    ADD_FIELD(RIC_V_EPOCH, u32, &data->tex_id);
}

static void cereal_s32(struct ric_config *config, s32 *data)
{
    FIELD_RW(data, sizeof(s32), 1, config->fp);
}

static void cereal_u32(struct ric_config *config, u32 *data)
{
    FIELD_RW(data, sizeof(u32), 1, config->fp);
}

static void cereal_float(struct ric_config *config, float *data)
{
    FIELD_RW(data, sizeof(float), 1, config->fp);
}

static void cereal_bool(struct ric_config *config, bool *data)
{
    FIELD_RW(data, sizeof(bool), 1, config->fp);
}

static void cereal_string(struct ric_config *config, char *data)
{
    RICO_ASSERT(0); // Strings not supported yet
    //FIELD_RW(data, ???, 1, config->fp);
}

static void cereal_test_1()
{
    struct ric_config config = { 0 };
    config.version = 1;
    config.write = false;
    config.fp = fopen("cereal_test_1.bin", config.write ? "wb" : "rb");
    RICO_ASSERT(config.fp);

    struct ric_data read = { 0 };
    cereal_data(&config, &read);
    fclose(config.fp);

    for (u32 i = 0; i < ARRAY_COUNT(read.entities); ++i)
    {
        RICO_ASSERT(read.entities[i].flag1 == false);
        RICO_ASSERT(read.entities[i].tex_id == i + 1);
    }
}

static void cereal_test_current()
{
    struct ric_data write = { 0 };
    for (u32 i = 0; i < ARRAY_COUNT(write.entities); ++i)
    {
        write.entities[i].flag1 = (i % 2 == 0);
        write.entities[i].tex_id = i + 1;
    }

    struct ric_config config = { 0 };
    config.version = RIC_V_CURRENT;

    config.write = true;
    config.fp = fopen("cereal_test.bin", config.write ? "wb" : "rb");
    RICO_ASSERT(config.fp);
    cereal_data(&config, &write);
    fclose(config.fp);

    struct ric_data read = { 0 };

    config.write = false;
    config.fp = fopen("cereal_test.bin", config.write ? "wb" : "rb");
    RICO_ASSERT(config.fp);
    cereal_data(&config, &read);
    fclose(config.fp);

    for (u32 i = 0; i < ARRAY_COUNT(read.entities); ++i)
    {
        RICO_ASSERT(read.entities[i].flag1 == write.entities[i].flag1);
        RICO_ASSERT(read.entities[i].tex_id == write.entities[i].tex_id);
    }
}

static void cereal_test()
{
    cereal_test_1();
    cereal_test_current();
    RICO_ASSERT(1);
}