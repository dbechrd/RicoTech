#define BBOX_EPSILON 0.01f
const struct vec3 BBOX_EPSILON_TRANS =
    {{{ BBOX_EPSILON / 2, BBOX_EPSILON / 2, BBOX_EPSILON / 2 }}};

void bbox_init(struct bbox *bbox, struct vec3 min, struct vec3 max,
               struct vec4 color)
{
    bbox->min = min;
    bbox->max = max;
    bbox->color = color;
    bbox->selected = false;
}

void bbox_init_mesh(struct bbox *bbox, struct rico_mesh *mesh,
                    struct vec4 color)
{
    RICO_ASSERT(mesh->vertex_count);

    struct pbr_vertex *verts = mesh_vertices(mesh);
    struct vec3 min = verts[0].pos;
    struct vec3 max = verts[0].pos;

    // Find bounds of mesh
    for (u32 i = 1; i < mesh->vertex_count; ++i)
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