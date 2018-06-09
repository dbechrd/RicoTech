#ifndef RICO_INTERNAL_PROGRAM_H
#define RICO_INTERNAL_PROGRAM_H

enum program_type
{
    PROG_NULL,
    PROG_PBR,
    PROG_PRIM,
    PROG_TEXT,
    PROG_COUNT
};

struct program
{
    enum program_type type;
    u32 ref_count;
    GLuint gl_id;
};

typedef void(*program_attribs_helper)();
static program_attribs_helper program_attribs[PROG_COUNT];

///=============================================================================
//| PBR program
///=============================================================================
#define LOCATION_PBR_POSITION 0
#define LOCATION_PBR_UV       1
#define LOCATION_PBR_COLOR    2
#define LOCATION_PBR_NORMAL   3

struct pbr_vertex
{
    struct vec3 pos;
    struct vec2f uv;
    struct vec4 col;
    struct vec3 normal;
};

struct pbr_attrib
{
    GLint position; // vec3
    GLint uv;       // vec2
    GLint color;    // vec3
    GLint normal;   // vec3
};

struct pbr_camera
{
    GLint pos; // vec3
};

struct pbr_material
{
    // rgb: metallic ? specular.rgb : albedo.rgb
    //   a: metallic ?            1 : opacity
    GLint tex0; // sampler2D

    // r: metallic
    // g: roughness
    // b: ao
    // a: NOT USED
    GLint tex1; // sampler2D

    // rgb: emission color
    //   a: NOT USED
    GLint tex2; // sampler2D
};

struct pbr_light_point
{
    GLint pos;       // vec3
    GLint color;     // vec3
    GLint intensity; // float
};

struct pbr_program_vert
{
    // Vertex shader
    GLint scale_uv; // vec3
    GLint model;    // mat4
    GLint view;     // mat4
    GLint proj;     // mat4

    struct pbr_attrib attrs;
};

struct pbr_program_frag
{
    // Fragment shader
    struct pbr_camera camera;
    struct pbr_material material;
    struct pbr_light_point light;
};

struct pbr_program
{
    struct program program;
    struct pbr_program_vert vert;
    struct pbr_program_frag frag;
};

static void program_pbr_attribs();
static int make_program_pbr(struct pbr_program **_program);
static void free_program_pbr(struct pbr_program **program);

///=============================================================================
//| Primitive program
///=============================================================================
#define LOCATION_PRIM_POSITION 0
#define LOCATION_PRIM_UV       1
#define LOCATION_PRIM_COLOR    2

struct prim_vertex
{
    struct vec3 pos;
    struct vec2f uv;
    struct vec4 col;
};

struct prim_program_attrib
{
    GLint position; // vec3
    GLint uv;       // vec2
    GLint color;    // vec3
};

struct prim_program_vert
{
    // Vertex shader
    GLint model;    // mat4
    GLint view;     // mat4
    GLint proj;     // mat4

    struct prim_program_attrib attrs;
};

struct prim_program_frag
{
    // Fragment shader
    GLint color;  // vec4
    GLint tex;  // sampler2D
};

struct prim_program
{
    struct program program;
    struct prim_program_vert vert;
    struct prim_program_frag frag;
};

static void program_primitive_attribs();
static int make_program_primitive(struct prim_program **_program);
static void free_program_primitive(struct prim_program **program);

///=============================================================================
//| Text program
///=============================================================================
#define LOCATION_TEXT_POSITION 0
#define LOCATION_TEXT_UV       1
#define LOCATION_TEXT_COLOR    2

struct text_vertex
{
    struct vec2f pos;
    struct vec2f uv;
    struct vec4 col;
};

struct text_program_attrib
{
    GLint position; // vec3
    GLint uv;       // vec2
    GLint color;    // vec3
};

struct text_program_vert
{
    // Vertex shader
    GLint model; // mat4
    GLint proj;  // mat4

    struct text_program_attrib attrs;
};

struct text_program_frag
{
    // Fragment shader
    GLint tex;   // sampler2D
};

struct text_program
{
    struct program program;
    struct text_program_vert vert;
    struct text_program_frag frag;
};

static void program_text_attribs();
static int make_program_text(struct text_program **_program);
static void free_program_text(struct text_program **program);

#endif