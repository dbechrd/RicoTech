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

struct pbr_program
{
    struct program program;
    
    // Vertex shader
    struct
    {
        GLint scale_uv; // vec3
        GLint model;    // mat4
        GLint view;     // mat4
        GLint proj;     // mat4

        // Vertex attributes
        struct
        {
            GLint position; // vec3
            GLint uv;       // vec2
            GLint color;    // vec3
            GLint normal;   // vec3
        }
        attrs;
    }
    vert;
    
    // Fragment shader
    struct
    {
        // Camera
        struct
        {
            GLint pos; // vec3
        }
        camera;

        // Material
        struct
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
        }
        material;

        // Light
        struct
        {
            GLint pos;       // vec3
            GLint color;     // vec3
            GLint intensity; // float
        }
        light;
    }
    frag;
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

struct prim_program
{
    struct program program;
    
    // Vertex shader
    struct
    {
        GLint model;    // mat4
        GLint view;     // mat4
        GLint proj;     // mat4
        
        // Vertex attributes
        struct
        {
            GLint position; // vec3
            GLint uv;       // vec2
            GLint color;    // vec3
        }
        attrs;
    }
    vert;
    
    // Fragment shader
    struct
    {
        GLint color;  // vec4
        GLint tex;  // sampler2D
    }
    frag;
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
    struct vec3 pos;
    struct vec2f uv;
    struct vec4 col;
};

struct text_program
{
    struct program program;
    
    // Vertex shader
    struct
    {
        GLint model; // mat4
        GLint view;  // mat4
        GLint proj;  // mat4
        
        // Vertex attributes
        struct
        {
            GLint position; // vec3
            GLint uv;       // vec2
            GLint color;    // vec3
        }
        attrs;
    }
    vert;

    // Fragment shader
    struct
    {
        GLint color;     // vec4
        GLint grayscale; // bool
        GLint tex;       // sampler2D
    }
    frag;
};

static void program_text_attribs();
static int make_program_text(struct text_program **_program);
static void free_program_text(struct text_program **program);

#endif