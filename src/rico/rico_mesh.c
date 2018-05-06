static void *mesh_vertices(struct RICO_mesh *mesh)
{
    return ((u8 *)mesh + mesh->vertices_offset);
}
static u32 *mesh_elements(struct RICO_mesh *mesh)
{
    return (u32 *)((u8 *)mesh + mesh->elements_offset);
}
static void mesh_upload(struct RICO_mesh *mesh, GLenum hint,
                 enum program_type prog_type)
{
#if RICO_DEBUG_MESH
    printf("[mesh][upld] name=%s\n", mesh_name(mesh));
#endif

    struct rgl_mesh rgl_mesh = { 0 };
    rgl_mesh.vertices = mesh->vertex_count;
    rgl_mesh.elements = mesh->element_count;

    //--------------------------------------------------------------------------
    // Generate VAO and buffers
    //--------------------------------------------------------------------------
    glGenVertexArrays(1, &rgl_mesh.vao);
    glBindVertexArray(rgl_mesh.vao);
    glGenBuffers(2, rgl_mesh.vbos);

    //--------------------------------------------------------------------------
    // Vertex buffer
    //--------------------------------------------------------------------------
    glBindBuffer(GL_ARRAY_BUFFER, rgl_mesh.vbos[VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER,
                 rgl_mesh.vertices * mesh->vertex_size,
                 mesh_vertices(mesh), hint);

    //--------------------------------------------------------------------------
    // Element buffer
    //--------------------------------------------------------------------------
    if (rgl_mesh.elements)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rgl_mesh.vbos[VBO_ELEMENT]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     rgl_mesh.elements * sizeof(u32),
                     mesh_elements(mesh), hint);
    }

    RICO_ASSERT(program_attribs[prog_type]);
    if(program_attribs[prog_type])
        program_attribs[prog_type]();

    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Store in hash table
    hashtable_insert_pkid(&global_meshes, mesh->uid.pkid, &rgl_mesh,
                          sizeof(rgl_mesh));
}
static void mesh_delete(struct RICO_mesh *mesh)
{
    struct rgl_mesh *rgl_mesh = hashtable_search_pkid(&global_meshes,
                                                      mesh->uid.pkid);
    if (!rgl_mesh) return;

#if RICO_DEBUG_MESH
    printf("[mesh][ del] name=%s\n", mesh_name(mesh));
#endif

    glDeleteBuffers(2, rgl_mesh->vbos);
    glDeleteVertexArrays(1, &rgl_mesh->vao);

    hashtable_delete_pkid(&global_meshes, mesh->uid.pkid);
}
static void mesh_render(pkid pkid, enum program_type prog_type)
{
    struct rgl_mesh *rgl_mesh = hashtable_search_pkid(&global_meshes, pkid);
    if (!rgl_mesh)
    {
        struct RICO_mesh *mesh = RICO_pack_lookup(pkid);
        RICO_ASSERT(mesh);
        mesh_upload(mesh, GL_STATIC_DRAW, prog_type);
        rgl_mesh = hashtable_search_pkid(&global_meshes, pkid);
    }

    // Draw
    RICO_ASSERT(rgl_mesh->vao);
    glBindVertexArray(rgl_mesh->vao);
    if (rgl_mesh->elements)
    {
        glDrawElements(GL_TRIANGLES, rgl_mesh->elements, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, rgl_mesh->vertices);
    }
    glBindVertexArray(0);
}