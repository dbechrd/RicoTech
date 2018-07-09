#version 330 core

out vs_out {
    vec3 P;
    vec2 UV;
    vec4 color;
    vec3 N;
} vertex;

uniform vec2 scale_uv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

layout(location = 0) in vec3 attr_position;
layout(location = 1) in vec2 attr_uv;
layout(location = 2) in vec4 attr_color;
layout(location = 3) in vec3 attr_normal;

void main()
{
    vertex.P =  vec3(model * vec4(attr_position, 1.0));
    vertex.UV = scale_uv * attr_uv;
    vertex.color = attr_color;
    vertex.N = transpose(inverse(mat3(model))) * attr_normal;
    gl_Position = proj * view * vec4(vertex.P, 1.0);
}