#version 330 core

out vs_out {
    vec3 P;
} vertex;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform vec3 p0;
uniform vec3 p1;

layout(location = 0) in vec3 attr_position;

void main()
{
    gl_Position = model * vec4(attr_position, 1.0);
    vertex.P = gl_Position.xyz;
    gl_Position = proj * view * gl_Position;
}