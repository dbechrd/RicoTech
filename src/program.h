#ifndef PROGRAM_H
#define PROGRAM_H

enum program_type
{
    PROG_NULL,
    PROG_PBR,
    PROG_PRIM,
    PROG_TEXT,
    PROG_COUNT
};

typedef void(*program_attribs_helper)();
extern program_attribs_helper program_attribs[PROG_COUNT];

///=============================================================================
//| PBR program
///=============================================================================
#define LOCATION_PBR_POSITION 0
#define LOCATION_PBR_COLOR    1
#define LOCATION_PBR_NORMAL   2
#define LOCATION_PBR_UV       3

struct pbr_vertex
{
    struct vec3 pos;
    struct vec4 col;
    struct vec3 normal;
    struct vec2 uv;
};

struct pbr_attrib
{
    GLint position; // vec3
    GLint color;    // vec3
    GLint normal;   // vec3
    GLint uv;       // vec2
};

struct pbr_camera
{
    GLint pos; // (vec3)
};

struct pbr_material
{
    // rgb: metallic ? specular.rgb : albedo.rgb
    //   a: metallic ?            1 : opacity
    GLint tex0; // (sampler2D)

    // r: metallic
    // g: roughness
    // b: ao
    // a: UNUSED
    GLint tex1; // (sampler2D)

    // rgb: emission color
    //   a: UNUSED
    GLint tex2; // (sampler2D)
};

struct pbr_light_point
{
    GLint pos;       // (vec3)
    GLint color;     // (vec3)
    GLint intensity; // (float)
};

struct program_pbr
{
    enum program_type type;
    u32 ref_count;
    GLuint prog_id;

    //TODO: Don't set from outside, create wrapper methods to enforce type
    // Vertex shader
    GLint time;     // float
    GLint scale_uv; // vec3
    GLint model;    // mat4
    GLint view;     // mat4
    GLint proj;     // mat4

    struct pbr_attrib attrs;

    // Fragment shader
    struct pbr_camera camera;
    struct pbr_material material;
    struct pbr_light_point light;
};

inline void program_pbr_attribs();
int make_program_pbr(struct program_pbr **_program);
void free_program_pbr(struct program_pbr **program);

///=============================================================================
//| Primitive program
///=============================================================================
#define LOCATION_PRIM_POSITION 0
#define LOCATION_PRIM_COLOR    1

struct prim_attrib
{
    GLint position; // vec3
    GLint color;    // vec3
};

struct program_primitive
{
    enum program_type type;
    GLuint prog_id;

    // Vertex shader
    GLint u_model;  //mat4
    GLint u_view;   //mat4
    GLint u_proj;   //mat4

    struct prim_attrib attrs;

    // Fragment shader
    GLint u_col;    //vec3
};

inline void program_primitive_attribs();
int make_program_primitive(struct program_primitive **_program);
void free_program_primitive(struct program_primitive **program);

///=============================================================================
//| Primitive cube program
///=============================================================================
struct program_prim_cube_attrib
{
    GLint position; // vec3
};

struct program_prim_cube
{
    enum program_type type;
    GLuint prog_id;

    // Vertex shader
    GLint model; // mat4
    GLint view;  // mat4
    GLint proj;  // mat4

    GLint p0;    //vec3
    GLint p1;    //vec3

    struct program_prim_cube_attrib attrs;

    // Fragment shader
    GLint color; //vec3
};

int make_program_prim_cube(struct program_prim_cube **_program);
void free_program_prim_cube(struct program_prim_cube **program);

///=============================================================================
//| Text program
///=============================================================================
#define LOCATION_TEXT_POSITION 0
#define LOCATION_TEXT_COLOR    1
#define LOCATION_TEXT_UV       2

struct text_vertex
{
    struct vec3 pos;
    struct vec4 col;
    struct vec2 uv;
};

struct program_text_attrib
{
    GLint position; // vec3
    GLint color;    // vec3
    GLint uv;       // vec2
};

struct program_text
{
    enum program_type type;
    GLuint prog_id;

    // Vertex shader
    GLint model; // mat4

    struct program_text_attrib attrs;

    // Fragment shader
    GLint tex;   // (sampler2D)
};

void program_text_attribs();
int make_program_text(struct program_text **_program);
void free_program_text(struct program_text **program);

#endif