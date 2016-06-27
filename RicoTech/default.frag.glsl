#version 330 core

in vec4 frag_col;
in vec2 frag_uv;

out vec4 col;

uniform sampler2D u_tex;

void main()
{
    col = texture(u_tex, frag_uv);// + frag_col;
}