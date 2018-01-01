#ifndef PROGRAM_H
#define PROGRAM_H

//--------------------------------------------------------------------------
// PBR program
//--------------------------------------------------------------------------
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
    GLint time;       // float
    GLint scale_uv;   // vec3
    GLint model;      // mat4
    GLint view;       // mat4
    GLint projection; // mat4

    struct pbr_attrib attrs;

    // Fragment shader
    struct pbr_camera camera;
    struct pbr_material material;
    struct pbr_light_point light;
};

//--------------------------------------------------------------------------
// Default program
//--------------------------------------------------------------------------
struct glsl_attrib
{
    GLint position; // vec3
    GLint normal;   // vec3
    GLint color;    // vec3
    GLint uv;       // vec2
};

struct glsl_camera
{
    GLint position; // (vec3)
};

struct glsl_material
{
    GLint diffuse;  // (sampler2D)
    GLint specular; // (sampler2D)
    GLint shiny;    // (float)
};

// Point light
struct glsl_light_point
{
    GLint position; // (vec3)

    // Color
    GLint ambient;  // (vec3)
    GLint color;    // (vec3)

    // Attenuation
    GLint kc;       // (float) Constant
    GLint kl;       // (float) Linear
    GLint kq;       // (float) Quadratic
};

struct program_default
{
    u32 ref_count;
    GLuint prog_id;

    //TODO: Don't set from outside, create wrapper methods to enforce type
    // Vertex shader
    GLint u_time;       // float
    GLint u_scale_uv;   // vec3
    GLint u_model;      // mat4
    GLint u_view;       // mat4
    GLint u_projection; // mat4

    struct glsl_attrib u_attr;

    // Fragment shader
    struct glsl_camera u_camera;
    struct glsl_material u_material;
    struct glsl_light_point u_light_point;
};

//int make_program_default(struct program_default **_program);
//void free_program_default(struct program_default **program);
//void program_default_uniform_projection(struct program_default *program,
//                                        struct mat4 *proj);

//--------------------------------------------------------------------------
// Primitive program
//--------------------------------------------------------------------------
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

//int make_program_primitive(struct program_primitive **_program);
//void free_program_primitive(struct program_primitive **program);
//void program_primitive_uniform_projection(struct program_primitive *program,
//                                          struct mat4 *proj);

#endif // PROGRAM_H
