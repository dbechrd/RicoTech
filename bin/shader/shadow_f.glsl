#version 330 core

in gs_out {
    vec4 P;
} geometry;

uniform float far_plane;
uniform vec3 light_pos;

void main()
{
    // Calculate distance from light to cubemap face
    //float dist = length(geometry.P.xyz - light_pos);
    float dist = distance(geometry.P.xyz, light_pos);

    // Normalize to 0..1
    dist = dist / far_plane;

    // Update depth bufferw
    gl_FragDepth = dist;
}