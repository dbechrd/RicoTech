#version 330 core

uniform vec4 u_col;

in vec4 frag_col;
out vec4 col;

void main()
{
    //col = vec4(u_col.rgb, frag_col.a * u_col.a);
    col = vec4(frag_col * u_col);
}