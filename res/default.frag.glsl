#version 330 core

in vec4 vtx_col;
in vec2 vtx_uv;

uniform sampler2D u_tex;

out vec4 col;

void main()
{
    float ambientStrength = 0.1f;
    vec4 ambient = vec4(0.7, 0.6, 0.4, 1.0);
    col = ambient * texture(u_tex, vtx_uv);// + vtx_col;
}