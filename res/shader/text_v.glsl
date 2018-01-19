#version 330 core

out vs_out {
    vec3 P;
    vec3 N;
    vec2 UV;
    vec4 color;
} vertex;

uniform mat4 model;

layout(location = 0) in vec3 attr_position;
layout(location = 1) in vec4 attr_color;
layout(location = 2) in vec2 attr_uv;

void main()
{
    vertex.color = attr_color;
    vertex.N = vec3(0.0, 0.0, 1.0);
    vertex.UV = attr_uv;
    gl_Position = model * vec4(attr_position, 1.0);
    vertex.P = gl_Position.xyz;
}