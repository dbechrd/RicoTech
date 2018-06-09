static struct rgl_mesh mesh_pool[256];
static struct rgl_mesh *mesh_freelist;

extern void rico_mesh_init()
{
    for (u32 i = 0; i < ARRAY_COUNT(mesh_pool) - 1; ++i)
    {
        mesh_pool[i].next = &mesh_pool[i + 1];
    }
    mesh_freelist = mesh_pool;
}
static void *mesh_vertices(struct RICO_mesh *mesh)
{
    return ((u8 *)mesh + mesh->vertices_offset);
}
static u32 *mesh_elements(struct RICO_mesh *mesh)
{
    return (u32 *)((u8 *)mesh + mesh->elements_offset);
}
static void mesh_upload(struct RICO_mesh *mesh, GLenum hint)
{
#if RICO_DEBUG_MESH
    printf("[mesh][upld] name=%s\n", mesh_name(mesh));
#endif

    RICO_ASSERT(mesh_freelist);
    struct rgl_mesh *rgl_mesh = mesh_freelist;
    mesh_freelist = rgl_mesh->next;

    rgl_mesh->vertices = mesh->vertex_count;
    rgl_mesh->elements = mesh->element_count;

    //--------------------------------------------------------------------------
    // Generate VAO and buffers
    //--------------------------------------------------------------------------
    glGenVertexArrays(1, &rgl_mesh->vao);
    glBindVertexArray(rgl_mesh->vao);
    glGenBuffers(2, rgl_mesh->vbos);

    //--------------------------------------------------------------------------
    // Vertex buffer
    //--------------------------------------------------------------------------
    glBindBuffer(GL_ARRAY_BUFFER, rgl_mesh->vbos[VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, rgl_mesh->vertices * mesh->vertex_size,
                 mesh_vertices(mesh), hint);

    //--------------------------------------------------------------------------
    // Element buffer
    //--------------------------------------------------------------------------
    if (rgl_mesh->elements)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rgl_mesh->vbos[VBO_ELEMENT]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, rgl_mesh->elements * sizeof(u32),
                     mesh_elements(mesh), hint);
    }

    RICO_ASSERT(program_attribs[mesh->prog_type]);
    if(program_attribs[mesh->prog_type])
        program_attribs[mesh->prog_type]();

    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Store in hash table
    hashtable_insert(&global_meshes, mesh->uid.pkid, rgl_mesh);
}
static void mesh_delete(struct RICO_mesh *mesh)
{
    struct rgl_mesh *rgl_mesh = hashtable_search(&global_meshes,
                                                 mesh->uid.pkid);
    if (!rgl_mesh) return;

#if RICO_DEBUG_MESH
    printf("[mesh][ del] name=%s\n", mesh_name(mesh));
#endif

    glDeleteBuffers(2, rgl_mesh->vbos);
    glDeleteVertexArrays(1, &rgl_mesh->vao);

    hashtable_delete(&global_meshes, mesh->uid.pkid);

    rgl_mesh->next = mesh_freelist;
    mesh_freelist = rgl_mesh;
}
static struct rgl_mesh *mesh_rgl(pkid pkid)
{
    struct rgl_mesh *rgl_mesh = hashtable_search(&global_meshes, pkid);
    if (!rgl_mesh)
    {
        struct RICO_mesh *mesh = RICO_pack_lookup(pkid);
        RICO_ASSERT(mesh);
        mesh_upload(mesh, GL_STATIC_DRAW);
        rgl_mesh = hashtable_search(&global_meshes, pkid);
        RICO_ASSERT(rgl_mesh);
    }
    return rgl_mesh;
}
static void mesh_bind_buffers(pkid pkid)
{
    struct rgl_mesh *rgl_mesh = mesh_rgl(pkid);
    glBindBuffer(GL_ARRAY_BUFFER, rgl_mesh->vbos[VBO_VERTEX]);
    if (rgl_mesh->elements)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rgl_mesh->vbos[VBO_ELEMENT]);
    }
}
static GLuint mesh_vao(pkid pkid)
{
    GLuint vao = mesh_rgl(pkid)->vao;
    return vao;
}
static void mesh_render(pkid pkid)
{
    // Draw
    struct rgl_mesh *rgl_mesh = mesh_rgl(pkid);
    if (rgl_mesh->elements)
    {
        glDrawElements(GL_TRIANGLES, rgl_mesh->elements, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, rgl_mesh->vertices);
    }
}
static void mesh_clusterfuck(pkid pkid)
{
    // Draw
    struct rgl_mesh *rgl_mesh = mesh_rgl(pkid);
    glBindBuffer(GL_ARRAY_BUFFER, rgl_mesh->vbos[VBO_VERTEX]);
    if (rgl_mesh->elements)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rgl_mesh->vbos[VBO_ELEMENT]);
    }
     
    glVertexAttribPointer(LOCATION_PRIM_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, pos));
    glEnableVertexAttribArray(LOCATION_PRIM_POSITION);

    glVertexAttribPointer(LOCATION_PRIM_UV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, uv));
    glEnableVertexAttribArray(LOCATION_PRIM_UV);

    glVertexAttribPointer(LOCATION_PRIM_COLOR, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct prim_vertex),
                          (GLvoid *)offsetof(struct prim_vertex, col));
    glEnableVertexAttribArray(LOCATION_PRIM_COLOR);

    if (rgl_mesh->elements)
    {
        glDrawElements(GL_TRIANGLES, rgl_mesh->elements, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, rgl_mesh->vertices);
    }
}