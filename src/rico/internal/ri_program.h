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
    struct vec2 uv;
    struct vec4 col;
    struct vec3 normal;
};

#define UNIFORM(type) GLint
struct pbr_program_locations
{
#   include "ri_progdef.h"
};
#undef UNIFORM

#define UNIFORM(type) type
struct pbr_program
{
    struct program program;
    struct pbr_program_locations locations;
#   include "ri_progdef.h"
};
#undef UNIFORM

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
    struct vec2 uv;
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
    struct vec2 uv;
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