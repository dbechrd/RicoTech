global const char *mesh_name(struct rico_mesh *mesh)
{
    RICO_ASSERT(mesh->name_offset);
    return (char *)((u8 *)mesh + mesh->name_offset);
}
global void *mesh_vertices(struct rico_mesh *mesh)
{
    return ((u8 *)mesh + mesh->vertices_offset);
}
internal u32 *mesh_elements(struct rico_mesh *mesh)
{
    return (u32 *)((u8 *)mesh + mesh->elements_offset);
}

void mesh_upload(struct rico_mesh *mesh, GLenum hint)
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

    //program_pbr_attribs();

    glVertexAttribPointer(LOCATION_PBR_POSITION, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, pos));
    glEnableVertexAttribArray(LOCATION_PBR_POSITION);
    
    glVertexAttribPointer(LOCATION_PBR_NORMAL, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, normal));
    glEnableVertexAttribArray(LOCATION_PBR_NORMAL);
    
    glVertexAttribPointer(LOCATION_PBR_COLOR, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, col));
    glEnableVertexAttribArray(LOCATION_PBR_COLOR);
    
    glVertexAttribPointer(LOCATION_PBR_UV, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct pbr_vertex),
                          (GLvoid *)offsetof(struct pbr_vertex, uv));
    glEnableVertexAttribArray(LOCATION_PBR_UV);

    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Store in hash table
    hashtable_insert_uid(&global_meshes, mesh->id, &rgl_mesh, sizeof(rgl_mesh));
}

void mesh_delete(struct rico_mesh *mesh)
{
    struct rgl_mesh *rgl_mesh = hashtable_search_uid(&global_meshes, mesh->id);
    if (!rgl_mesh) return;

#if RICO_DEBUG_MESH
    printf("[mesh][ del] name=%s\n", mesh_name(mesh));
#endif

    glDeleteBuffers(2, rgl_mesh->vbos);
    glDeleteVertexArrays(1, &rgl_mesh->vao);

    hashtable_delete_uid(&global_meshes, mesh->id);
}

void mesh_render(struct pack *pack, u32 id, enum rico_obj_type obj_type)
{
    RICO_ASSERT(pack);

    struct rgl_mesh *rgl_mesh = hashtable_search_uid(&global_meshes, id);
    if (!rgl_mesh)
    {
        struct rico_mesh *mesh = pack_lookup(pack, id);
        mesh_upload(mesh, GL_STATIC_DRAW);
        rgl_mesh = hashtable_search_uid(&global_meshes, id);
    }
    RICO_ASSERT(rgl_mesh);
    RICO_ASSERT(rgl_mesh->vao);

    // Draw
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
