#version 330 core

out vs_out {
    vec3 P;
    vec2 UV;
    vec4 color;
    vec3 N;
} vertex;

uniform vec2 scale_uv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

layout(location = 0) in vec3 attr_position;
layout(location = 1) in vec2 attr_uv;
layout(location = 2) in vec4 attr_color;
layout(location = 3) in vec3 attr_normal;

void main()
{
    vertex.color = attr_color;
    vertex.N = (mat3(transpose(inverse(model))) * attr_normal).xyz;
    vertex.UV = scale_uv * attr_uv;
    gl_Position = model * vec4(attr_position, 1.0);
    vertex.P = gl_Position.xyz;
    gl_Position = proj * view * gl_Position;
}