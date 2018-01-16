#version 330 core

in vs_out {
    vec3 P;
    vec3 N;
    vec2 UV;
    vec4 color;
} vertex;

out vec4 frag_color;

uniform sampler2D tex;

void main()
{
    vec4 color = texture(tex, vertex.UV);
    if (color.a == 0)
    {
        color = vertex.color;
    }
    frag_color = color;
}