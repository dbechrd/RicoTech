#version 330 core

uniform mat4 model;

layout(location = 0) in vec3 attr_position;

void main()
{
    gl_Position = model * vec4(attr_position, 1.0);
}