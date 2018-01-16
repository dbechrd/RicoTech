#ifndef PROGRAM_H
#define PROGRAM_H

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
    // Dielectic: albedo.rgb, a = opacity
    // Metallic:  specular.rgb, a = UNUSED
    GLint tex0; // (sampler2D)

    // r: metallic
    // g: roughness
    // b: ao
    // a: UNUSED
    GLint tex1; // (sampler2D)
};

struct pbr_light_point
{
    GLint pos;       // (vec3)
    GLint color;     // (vec3)
    GLint intensity; // (float)
};

struct program_pbr
{
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
struct program_primitive
{
    GLuint prog_id;

    // Vertex shader
    GLint u_model;  //mat4
    GLint u_view;   //mat4
    GLint u_proj;   //mat4

    GLint vert_pos; //vec3
    GLint vert_col; //vec3

    // Fragment shader
    GLint u_col;    //vec3
};

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
    GLuint prog_id;

    // Vertex shader
    GLint model; // mat4

    struct program_text_attrib attrs;

    // Fragment shader
    GLint color; // vec3
    GLint tex;   // (sampler2D)
};

int make_program_text(struct program_text **_program);
void free_program_text(struct program_text **program);

#endif