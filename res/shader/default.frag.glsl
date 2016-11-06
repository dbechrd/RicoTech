#version 330 core

uniform vec4 u_ambient;
uniform sampler2D u_tex;

in vec4 vtx_col;
in vec2 vtx_uv;

out vec4 col;

void main()
{
    col = u_ambient * texture(u_tex, vtx_uv);// + vtx_col;
}