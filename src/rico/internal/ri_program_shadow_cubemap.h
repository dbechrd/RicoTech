// Vertex shader
struct
{
    UNIFORM(struct mat4) model;

    // Vertex attributes
    struct
    {
        UNIFORM(struct vec3) position;
    }
    attrs;
}
vert;

// Geometry shader
struct
{
    UNIFORM(struct mat4) cubemap_xforms[6];
}
geom;

// Fragment shader
struct
{
    UNIFORM(struct vec2) near_far;
    UNIFORM(struct vec3) light_pos;
    UNIFORM(struct vec3) light_dir;
}
frag;