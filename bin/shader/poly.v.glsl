#version 150

in vec3 position;

void main()
{
    //mat4 window_scale = mat4(
    //    vec3(3.0/4.0, 0.0, 0.0, 0.0),
    //    vec3(    0.0, 1.0, 0.0, 0.0),
    //    vec3(    0.0, 0.0, 1.0, 0.0),
    //    vec3(    0.0, 0.0, 0.0, 1.0)
    //);
    mat4 window_scale = mat4(
        vec3(2.0/1024.0,       0.0, 0.0, 0.0),
        vec3(      0.0,  2.0/768.0, 0.0, 0.0),
        vec3(      0.0,        0.0, 1.0, 0.0),
        vec3(      0.0,        0.0, 0.0, 1.0)
    );
    gl_Position = vec3(window_scale * position);
}