#version 150

in vec4 position;

void main()
{
    //mat4 window_scale = mat4(
    //    vec4(3.0/4.0, 0.0, 0.0, 0.0),
    //    vec4(    0.0, 1.0, 0.0, 0.0),
    //    vec4(    0.0, 0.0, 1.0, 0.0),
    //    vec4(    0.0, 0.0, 0.0, 1.0)
    //);
    mat4 window_scale = mat4(
        vec4(1.0/512.0,       0.0, 0.0, 0.0),
        vec4(      0.0, 1.0/384.0, 0.0, 0.0),
        vec4(      0.0,       0.0, 1.0, 0.0),
        vec4(      0.0,      -1.0, 0.0, 1.0)
    );
    gl_Position = vec4(window_scale * position);
}