#version 330 core

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec4 vert_col;

out vec4 frag_col;

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(vert_pos, 1.0f);
    frag_col = vert_col;
}