#define BBOX_EPSILON 0.01f
const struct vec3 BBOX_EPSILON_TRANS =
    {{{ BBOX_EPSILON / 2, BBOX_EPSILON / 2, BBOX_EPSILON / 2 }}};

void bbox_init(struct bbox *bbox, struct vec3 min, struct vec3 max,
               struct vec4 color)
{
    bbox->min = min;
    bbox->max = max;
    bbox->color = color;
    bbox->wireframe = true;
}

void bbox_init_mesh(struct bbox *bbox, struct rico_mesh *mesh,
                    struct vec4 color)
{
    struct vec3 min = VEC3(9999.0, 9999.0, 9999.0);
    struct vec3 max = VEC3_ZERO;

    // Find bounds of mesh
    struct pbr_vertex *verts = mesh_vertices(mesh);
    for (u32 i = 0; i < mesh->vertex_count; ++i)
    {
        if (verts[i].pos.x > max.x) max.x = verts[i].pos.x;
        if (verts[i].pos.x < min.x) min.x = verts[i].pos.x;

        if (verts[i].pos.y > max.y) max.y = verts[i].pos.y;
        if (verts[i].pos.y < min.y) min.y = verts[i].pos.y;

        if (verts[i].pos.z > max.z) max.z = verts[i].pos.z;
        if (verts[i].pos.z < min.z) min.z = verts[i].pos.z;
    }

    // Prevent infinitesimally small bounds
    max.x += BBOX_EPSILON;
    min.x -= BBOX_EPSILON;
    max.y += BBOX_EPSILON;
    min.y -= BBOX_EPSILON;
    max.z += BBOX_EPSILON;
    min.z -= BBOX_EPSILON;

    bbox_init(bbox, min, max, color);
}