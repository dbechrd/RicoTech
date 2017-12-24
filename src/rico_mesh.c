const char *rico_mesh_type_string[] = {
    RICO_MESH_TYPES(GEN_STRING)
};

global const char *mesh_name(struct rico_mesh *mesh)
{
    return (u8 *)mesh + mesh->name_offset;
}
internal struct mesh_vertex *mesh_vertices(struct rico_mesh *mesh)
{
    return (struct mesh_vertex *)((u8 *)mesh + mesh->vertices_offset);
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

    //--------------------------------------------------------------------------
    // Generate VAO and buffers
    //--------------------------------------------------------------------------
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);
    glGenBuffers(2, mesh->vbos);

    //--------------------------------------------------------------------------
    // Vertex buffer
    //--------------------------------------------------------------------------
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh->vertex_count * sizeof(struct rico_vertex),
                 mesh_vertices(mesh), hint);

    //--------------------------------------------------------------------------
    // Element buffer
    //--------------------------------------------------------------------------
    if (mesh->element_count)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbos[VBO_ELEMENT]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     mesh->element_count * sizeof(u32),
                     mesh_elements(mesh), hint);
    }

    //--------------------------------------------------------------------------
    // Shader attribute pointers
    //--------------------------------------------------------------------------
    glVertexAttribPointer(RICO_SHADER_POS_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct rico_vertex),
                          (GLvoid *)offsetof(struct rico_vertex, pos));
    glEnableVertexAttribArray(RICO_SHADER_POS_LOC);

    glVertexAttribPointer(RICO_SHADER_NORMAL_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct rico_vertex),
                          (GLvoid *)offsetof(struct rico_vertex, normal));
    glEnableVertexAttribArray(RICO_SHADER_NORMAL_LOC);

    glVertexAttribPointer(RICO_SHADER_COL_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct rico_vertex),
                          (GLvoid *)offsetof(struct rico_vertex, col));
    glEnableVertexAttribArray(RICO_SHADER_COL_LOC);

    glVertexAttribPointer(RICO_SHADER_UV_LOC, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct rico_vertex),
                          (GLvoid *)offsetof(struct rico_vertex, uv));
    glEnableVertexAttribArray(RICO_SHADER_UV_LOC);

    // Clean up
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    mesh->loaded = true;
}

void mesh_delete(struct rico_mesh *mesh)
{
#if RICO_DEBUG_MESH
    printf("[mesh][ del] name=%s\n", mesh_name(mesh));
#endif

    mesh->loaded = false;
    glDeleteBuffers(2, mesh->vbos);
    glDeleteVertexArrays(1, &mesh->vao);
}

void mesh_render(struct pack *pack, u32 id)
{
    RICO_ASSERT(pack);
    RICO_ASSERT(id < pack->blobs_used);

    struct rico_mesh *mesh = pack_read(pack, id);
    if (!mesh->loaded)
    {
        mesh_upload(mesh, GL_STATIC_DRAW);
    }

    // Draw
    glBindVertexArray(mesh->vao);
    if (mesh->element_count)
    {
        glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
    }
    glBindVertexArray(0);
}
