#version 330 core

uniform float u_time;
uniform vec2 u_scale_uv;
uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

layout(location = 0) in vec3 attr_position;
layout(location = 1) in vec4 attr_color;
layout(location = 2) in vec3 attr_normal;
layout(location = 3) in vec2 attr_uv;

out vert_out {
    vec3 position;
    vec4 color;
    vec3 normal;
    vec2 uv;
} vert;

void main()
{
    vert.color = attr_color;
    vert.normal = (mat3(transpose(inverse(u_model))) * attr_normal).xyz;
    vert.uv = u_scale_uv * attr_uv;
    gl_Position = u_model * vec4(attr_position, 1.0);
    vert.position = gl_Position.xyz;
    gl_Position = u_proj * u_view * gl_Position;
}