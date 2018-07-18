// Vertex shader
struct
{
    UNIFORM(struct vec3) scale_uv;
    UNIFORM(struct mat4) model;
    UNIFORM(struct mat4) view;
    UNIFORM(struct mat4) proj;
    UNIFORM(struct mat4) light_space;
    //TODO: UNIFORM(struct mat4) light_space[NUM_LIGHT_DIR];

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
    UNIFORM(float) time;

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
    struct NO_UNIFORM(pbr_light)
    {
        NO_UNIFORM(enum pbr_light_type type;)
        UNIFORM(struct vec3) pos;
        UNIFORM(struct vec3) dir;
        UNIFORM(struct vec3) color;
        UNIFORM(float)       intensity;
        UNIFORM(bool)        enabled;
    }
    lights[NUM_LIGHT_DIR + NUM_LIGHT_POINT];

    // Shadows
    UNIFORM(struct vec2) near_far;
    UNIFORM(GLint)       shadow_textures[NUM_LIGHT_DIR];
    UNIFORM(GLint)       shadow_cubemaps[NUM_LIGHT_POINT];
    UNIFORM(struct mat4) light_proj;
}
frag;