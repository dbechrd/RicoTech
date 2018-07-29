// Vertex shader
struct
{
    UNIFORM(struct mat4) model;
    UNIFORM(struct mat4) view;
    UNIFORM(struct mat4) proj;

    // Vertex attributes
    struct
    {
        UNIFORM(struct vec3) position;
        UNIFORM(struct vec2) uv;
        UNIFORM(struct vec3) color;
    }
    attrs;
}
vert;

// Fragment shader
struct
{
    UNIFORM(struct vec4) color;
    UNIFORM(GLint)       tex0;
}
frag;