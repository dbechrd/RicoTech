#define BBOX_EPSILON 0.001f
static const struct vec3 BBOX_EPSILON_TRANS =
    {{{ BBOX_EPSILON / 2, BBOX_EPSILON / 2, BBOX_EPSILON / 2 }}};

static void bbox_init(struct RICO_bbox *bbox, struct vec3 min, struct vec3 max)
{
    bbox->min = min;
    bbox->max = max;
}
static void bbox_init_mesh(struct RICO_bbox *bbox, struct RICO_mesh *mesh)
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

    bbox_init(bbox, min, max);
}
void RICO_bbox_transform(struct RICO_bbox *bbox, const struct mat4 *m)
{
    v3_mul_mat4(&bbox->min, m);
    v3_mul_mat4(&bbox->max, m);

    if (bbox->min.x > bbox->max.x) swapf(&bbox->min.x, &bbox->max.x);
    if (bbox->min.y > bbox->max.y) swapf(&bbox->min.y, &bbox->max.y);
    if (bbox->min.z > bbox->max.z) swapf(&bbox->min.z, &bbox->max.z);
}