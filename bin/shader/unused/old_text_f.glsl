#version 330 core

in vs_out {
    vec3 P;
    vec2 UV;
    vec4 color;
} vertex;

uniform sampler2D tex;

out vec4 frag_color;

void main()
{
    vec4 texel = texture(tex, vertex.UV);
    frag_color = mix(vertex.color, texel, step(0.01, texel.a));
}