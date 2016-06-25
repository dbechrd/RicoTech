#version 330 core

layout(location = 0) in vec4 vert_pos;
layout(location = 1) in vec4 vert_col;
layout(location = 2) in vec2 vert_uv;

out vec4 frag_col;
out vec2 frag_uv;

uniform float u_time;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(vert_pos);
    //gl_Position = vec4(position) * model * view * proj;
    frag_col = vert_col;
    frag_uv = vert_uv;
}