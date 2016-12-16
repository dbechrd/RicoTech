#version 330 core

uniform bool u_string;
uniform vec4 u_ambient;
uniform sampler2D u_tex;

in vec4 vtx_col;
in vec2 vtx_uv;

out vec4 col;

void main()
{
    //vec4 texCol = texture(u_tex, vtx_uv);
    vec4 texCol = vec4(0.0f);
    if (texCol.a == 0.0f) {
        col = vtx_col;
        col.a = 0.7f;
    } else {
        col = (u_ambient * texCol);
    }
}