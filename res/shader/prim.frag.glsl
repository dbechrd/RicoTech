#version 330 core

uniform vec4 u_col;

out vec4 frag_color;

void main()
{
    frag_color = u_col;
}