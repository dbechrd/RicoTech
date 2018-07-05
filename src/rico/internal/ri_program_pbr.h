// Vertex shader
struct
{
    UNIFORM(struct vec3) scale_uv;
    UNIFORM(struct mat4) model;
    UNIFORM(struct mat4) view;
    UNIFORM(struct mat4) proj;

    // Vertex attributes
    struct
    {
        UNIFORM(struct vec3) position;
        UNIFORM(struct vec2) uv;
        UNIFORM(struct vec3) color;
        UNIFORM(struct vec3) normal;
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
        UNIFORM(struct vec3) pos;
    }
    camera;

    // Material
    struct
    {
        // rgb: metallic ? specular.rgb : albedo.rgb
        //   a: metallic ?            1 : opacity
        UNIFORM(GLint) tex0; // sampler2D

        // r: metallic
        // g: roughness
        // b: ao
        // a: NOT USED
        UNIFORM(GLint) tex1; // sampler2D

        // rgb: emission color
        //   a: NOT USED
        UNIFORM(GLint) tex2; // sampler2D
    }
    material;

    // Light
    struct
    {
        UNIFORM(struct vec3) pos;
        UNIFORM(struct vec3) color;
        UNIFORM(float)       intensity;
        UNIFORM(bool)        enabled;
    }
    lights[NUM_LIGHTS];
}
frag;