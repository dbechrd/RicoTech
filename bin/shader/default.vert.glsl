#version 330 core

uniform float u_time;
uniform vec2 u_scale_uv;
uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec4 vert_col;
layout(location = 2) in vec3 vert_normal;
layout(location = 3) in vec2 vert_uv;

out vtx_out {
    vec4 col;
    vec3 normal;
    vec2 uv;
    vec3 frag_pos;
} vtx;

void main()
{
    vtx.col = vert_col;
    vtx.normal = (mat3(transpose(inverse(u_model))) * vert_normal).xyz;
    vtx.uv = u_scale_uv * vert_uv;
    gl_Position = u_model * vec4(vert_pos, 1.0);
    vtx.frag_pos = gl_Position.xyz;
    gl_Position = u_proj * u_view * gl_Position;
}