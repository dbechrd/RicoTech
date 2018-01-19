#version 330 core

in vs_out {
    vec3 P;
    vec3 N;
    vec2 UV;
    vec4 color;
} vertex;

uniform sampler2D tex;

void main()
{
    vec4 texel = texture(tex, vertex.UV);
    gl_FragColor = mix(vertex.color, texel, step(0.01, texel.a));
}