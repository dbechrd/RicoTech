#version 330 core

uniform float u_time;
uniform vec2 u_scale_uv;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec4 vert_col;
layout(location = 2) in vec2 vert_uv;

out vec4 vtx_col;
out vec2 vtx_uv;

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(vert_pos.xyz, 1.0f);
    //gl_Position = vec4(vert_pos.xyz, 1.0f) * u_model * u_view * u_proj;
    //vtx_col = vert_col;
    vtx_col = vec4(
        0.5 + (30.0f / vert_pos.x),
        0.5 + (30.0f / vert_pos.y),
        0.5 + (30.0f / vert_pos.z),
        1.0f
    );
    vtx_uv = u_scale_uv * vert_uv;
    /*
    vtx_uv = vec2(
        (1100.0f / vert_pos.x),
        (1100.0f / vert_pos.z)
    );
    */
}