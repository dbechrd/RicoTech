#version 330 core

layout(location = 0) in vec4 vert_pos;

out vec4 frag_col;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform vec4 u_color;

void main()
{
    gl_Position = u_proj * u_view * u_model * vert_pos;
    frag_col = u_color;
}