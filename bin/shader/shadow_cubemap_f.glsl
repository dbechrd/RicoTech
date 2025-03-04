#version 330 core

in gs_out {
    vec4 P;
} geometry;

uniform vec2 near_far;
uniform vec3 light_pos;

void main()
{
    // Calculate distance from light to cubemap face
    float dist = length(geometry.P.xyz - light_pos);

    // Normalize to 0..1
    //dist /= (near_far.y - near_far.x);
    dist /= near_far.y;

    // Update depth buffer
    gl_FragDepth = dist;
}