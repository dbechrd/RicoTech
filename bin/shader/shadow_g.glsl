#version 330 core

out gs_out {
    vec4 P;
} geometry;

uniform mat4 cubemap_xforms[6];

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for (int vert = 0; vert < 3; ++vert)
        {
            geometry.P = gl_in[vert].gl_Position;
            gl_Position = cubemap_xforms[gl_Layer] * geometry.P;
            EmitVertex();
        }
        EndPrimitive();
    }
}