#ifndef RICO_INTERNAL_PROGRAM_H
#define RICO_INTERNAL_PROGRAM_H

#define NUM_LIGHTS 4

enum program_type
{
    PROG_NULL,
    PROG_PBR,
    PROG_SHADOW,
    PROG_PRIM,
    PROG_TEXT,
    PROG_COUNT
};

typedef void(*program_attribs_helper)();
static program_attribs_helper program_attribs[PROG_COUNT];

struct program
{
    enum program_type type;
    u32 ref_count;
    GLuint gl_id;
};

#define UNIFORM(type) GLint
#define STRUCT_NAME(name)
struct pbr_program_locations
{
#   include "ri_program_pbr.h"
};

struct shadow_program_locations
{
#   include "ri_program_shadow.h"
};

struct text_program_locations
{
#   include "ri_program_text.h"
};

struct prim_program_locations
{
#   include "ri_program_prim.h"
};
#undef UNIFORM
#undef STRUCT_NAME

#define UNIFORM(type) type
#define STRUCT_NAME(name) name
struct pbr_program
{
    struct program program;
    struct pbr_program_locations locations;
#   include "ri_program_pbr.h"
};

struct shadow_program
{
    struct program program;
    struct shadow_program_locations locations;
#   include "ri_program_shadow.h"
};

struct text_program
{
    struct program program;
    struct text_program_locations locations;
#   include "ri_program_text.h"
};

struct prim_program
{
    struct program program;
    struct prim_program_locations locations;
#   include "ri_program_prim.h"
};
#undef UNIFORM
#undef STRUCT_NAME

#define LOCATION_PBR_POSITION 0
#define LOCATION_PBR_UV       1
#define LOCATION_PBR_COLOR    2
#define LOCATION_PBR_NORMAL   3

#define LOCATION_SHADOW_POSITION 0

#define LOCATION_PRIM_POSITION 0
#define LOCATION_PRIM_UV       1
#define LOCATION_PRIM_COLOR    2

#define LOCATION_TEXT_POSITION 0
#define LOCATION_TEXT_UV       1
#define LOCATION_TEXT_COLOR    2

struct pbr_vertex
{
    struct vec3 pos;
    struct vec2 uv;
    struct vec4 col;
    struct vec3 normal;
};

struct prim_vertex
{
    struct vec3 pos;
    struct vec2 uv;
    struct vec4 col;
};

struct text_vertex
{
    struct vec3 pos;
    struct vec2 uv;
    struct vec4 col;
};

static void program_pbr_attribs();
static int make_program_pbr(struct pbr_program **_program);
static void free_program_pbr(struct pbr_program **program);

static void program_shadow_attribs();
static int make_program_shadow(struct shadow_program **_program);
static void free_program_shadow(struct shadow_program **program);

static void program_primitive_attribs();
static int make_program_primitive(struct prim_program **_program);
static void free_program_primitive(struct prim_program **program);

static void program_text_attribs();
static int make_program_text(struct text_program **_program);
static void free_program_text(struct text_program **program);

#endif