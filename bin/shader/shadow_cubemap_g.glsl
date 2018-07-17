#version 330 core

out gs_out {
    vec4 P;
} geometry;

uniform mat4 cubemap_xforms[6];

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

void render_face(int face)
{
    gl_Layer = face;
    for (int vert = 0; vert < 3; ++vert)
    {
        geometry.P = gl_in[vert].gl_Position;
        gl_Position = cubemap_xforms[face] * geometry.P;
        EmitVertex();
    }
    EndPrimitive();
}

void main()
{
    //   0      1      2      3      4      5
    // pos_x, neg_x, pos_y, neg_y, pos_z, neg_z
    //render_face(3);

    for (int face = 0; face < 6; ++face)
    {
        render_face(face);
    }
}