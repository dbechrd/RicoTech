#define BBOX_EPSILON 0.001f

static void bbox_init_mesh(struct RICO_aabb *aabb, struct RICO_mesh *mesh)
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
    //max.x += BBOX_EPSILON;
    //min.x -= BBOX_EPSILON;
    //max.y += BBOX_EPSILON;
    //min.y -= BBOX_EPSILON;
    //max.z += BBOX_EPSILON;
    //min.z -= BBOX_EPSILON;

    aabb->e = VEC3(
        (max.x - min.x + BBOX_EPSILON) / 2.0f,
        (max.y - min.y + BBOX_EPSILON) / 2.0f,
        (max.z - min.z + BBOX_EPSILON) / 2.0f
    );
    aabb->c = VEC3(
        min.x + aabb->e.x,
        min.y + aabb->e.y,
        min.z + aabb->e.z
    );
}
#if 0
void RICO_aabb_transform(struct RICO_aabb *aabb, const struct mat4 *m)
{
    v3_mul_mat4(&aabb->min, m);
    v3_mul_mat4(&aabb->max, m);

    if (aabb->min.x > aabb->max.x) swap_r32(&aabb->min.x, &aabb->max.x);
    if (aabb->min.y > aabb->max.y) swap_r32(&aabb->min.y, &aabb->max.y);
    if (aabb->min.z > aabb->max.z) swap_r32(&aabb->min.z, &aabb->max.z);
}
#endif