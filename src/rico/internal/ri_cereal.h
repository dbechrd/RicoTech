#ifndef RICO_INTERNAL_CEREAL_H
#define RICO_INTERNAL_CEREAL_H

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

////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////

static size_t ric_fwrite(void const* buf, size_t size,
                         struct ric_stream *stream);
static size_t ric_fread(void *buf, size_t size, struct ric_stream *stream);
static void rem_field(struct ric_stream *stream, enum ric_version v_add,
                      enum ric_version v_remove, u32 size, void *field);
static void rem_field_ptr(struct ric_stream *stream, enum ric_version v_add,
                          enum ric_version v_remove, u32 size, void **field);
static void ric_uid(struct ric_stream *stream, struct uid *data);
static void ric_RICO_transform(struct ric_stream *stream,
                                 struct RICO_transform *data);
static void ric_RICO_object(struct ric_stream *stream,
                              struct RICO_object *data);
static void ric_RICO_scene(struct ric_stream *stream, struct RICO_scene *data);

#endif