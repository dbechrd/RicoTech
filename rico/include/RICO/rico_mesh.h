#ifndef RICO_MESH_H
#define RICO_MESH_H

struct RICO_mesh
{
    struct uid uid;
    u32 vertex_size;
    u32 vertex_count;
    u32 element_count;
    struct RICO_bbox RICO_bbox;

    u32 vertices_offset;
    u32 elements_offset;
};

#endif