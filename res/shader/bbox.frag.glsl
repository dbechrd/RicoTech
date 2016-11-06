#version 330 core

uniform vec4 u_ambient;

in vec4 frag_col;

out vec4 col;

void main()
{
    col = frag_col;
}