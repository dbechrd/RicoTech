// Vertex shader
struct
{
    UNIFORM(struct mat4) light_space;
    UNIFORM(struct mat4) model;

    // Vertex attributes
    struct
    {
        UNIFORM(struct vec3) position;
    }
    attrs;
}
vert;

// Fragment shader
//struct
//{
//}
//frag;