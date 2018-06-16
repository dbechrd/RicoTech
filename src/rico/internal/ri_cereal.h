#ifndef RICO_INTERNAL_CEREAL_H
#define RICO_INTERNAL_CEREAL_H

enum ric_version
{
    V_EPOCH = 1,

    V_NEXT
};
#define V_CURRENT (V_NEXT - 1)

struct ric_stream
{
    s32 version;
    u8 mode;
    FILE *fp;
    u32 length;
    void *data;
    u32 offset;
};

static void ric_s32(struct ric_stream *config, s32 *data);
static void ric_u32(struct ric_stream *config, u32 *data);
static void ric_float(struct ric_stream *config, float *data);
static void ric_bool(struct ric_stream *config, bool *data);
static void ric_enum(struct ric_stream *stream, void *data);
static void ric_vec3(struct ric_stream *stream, struct vec3 *data);
static void ric_quat(struct ric_stream *stream, struct quat *data);

static void ric_uid(struct ric_stream *stream, struct uid *data);
static void ric_RICO_transform(struct ric_stream *stream,
                               struct RICO_transform *data);
static void ric_RICO_object(struct ric_stream *stream,
                            struct RICO_object *data);
static void ric_RICO_scene(struct ric_stream *stream, struct RICO_scene *data);

#endif