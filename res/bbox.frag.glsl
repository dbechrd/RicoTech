#version 330 core

in vec4 frag_col;

out vec4 col;

void main()
{
    col = frag_col;
}