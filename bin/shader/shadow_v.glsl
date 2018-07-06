#version 330 core

uniform mat4 model;

layout(location = 0) in vec3 attr_position;
//layout(location = 1) in vec2 attr_uv;
//layout(location = 2) in vec4 attr_color;

void main()
{
    gl_Position = model * vec4(attr_position, 1.0);
}