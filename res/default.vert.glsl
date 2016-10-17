#version 330 core

layout(location = 0) in vec4 vert_pos;
layout(location = 1) in vec4 vert_col;
layout(location = 2) in vec2 vert_uv;

uniform float u_time;
uniform vec2 u_scale_uv;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec4 vtx_col;
out vec2 vtx_uv;

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(vert_pos);
    //gl_Position = vec4(vert_pos) * u_model * u_view * u_proj;
    vtx_col = vert_col;
    vtx_uv = u_scale_uv * vert_uv;
}