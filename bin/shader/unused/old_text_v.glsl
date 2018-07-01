#version 330 core

out vs_out {
    vec3 P;
    vec2 UV;
    vec4 color;
} vertex;

uniform mat4 proj;
uniform mat4 model;

layout(location = 0) in vec2 attr_position;
layout(location = 1) in vec2 attr_uv;
layout(location = 2) in vec4 attr_color;

void main()
{
    vertex.color = attr_color;
    vertex.UV = attr_uv;
    gl_Position = model * vec4(attr_position.xy, -1.0f, 1.0);
    vertex.P = gl_Position.xyz;
    gl_Position = proj * gl_Position;
}