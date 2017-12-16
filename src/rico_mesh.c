const char *rico_mesh_type_string[] = {
    RICO_MESH_TYPES(GEN_STRING)
};

struct pool_id RICO_DEFAULT_MESH;

internal int build_mesh(struct rico_mesh *mesh, u32 vertex_count,
                        const struct mesh_vertex *vertex_data,
                        u32 element_count, const GLuint *element_data,
                        GLenum hint);

bool mesh_selectable(struct rico_mesh *mesh)
{
    return (mesh->type != MESH_STRING_SCREEN);
}

struct rico_mesh *mesh_next(struct rico_mesh *mesh)
{
    return pool_next(mesh->hnd.pool, mesh);
}

struct rico_mesh *mesh_prev(struct rico_mesh *mesh)
{
    return pool_prev(mesh->hnd.pool, mesh);
}

int mesh_init(struct rico_mesh *mesh, const char *name,
              enum rico_mesh_type type, u32 vertex_count,
              const struct mesh_vertex *vertex_data, u32 element_count,
              const GLuint *element_data, GLenum hint)
{
#if RICO_DEBUG_MESH
    printf("[mesh][init] name=%s vertices=%d\n", name, vertex_count);
#endif

    enum rico_error err;

    // Note: If we want to serialize mesh data we have to store the vertex data
    //       and element array in the struct.
    hnd_init(&mesh->hnd, RICO_HND_MESH, name);
    mesh->type = type;

    err = build_mesh(mesh, vertex_count, vertex_data, element_count,
                     element_data, hint);
    if (err) return err;

    err = bbox_init_mesh(&mesh->bbox, name, vertex_data, vertex_count,
                         COLOR_WHITE_HIGHLIGHT);
    if (err) return err;

    // Store in global hash table
    err = hashtable_insert_hnd(&global_meshes, &mesh->hnd, mesh);
    return err;
}

internal int build_mesh(struct rico_mesh *mesh, u32 vertex_count,
                        const struct mesh_vertex *vertex_data,
                        u32 element_count, const GLuint *element_data,
                        GLenum hint)
{
    mesh->element_count = element_count;

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
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(struct mesh_vertex),
                 vertex_data, hint);

    //--------------------------------------------------------------------------
    // Element buffer
    //--------------------------------------------------------------------------
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbos[VBO_ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_count * sizeof(GLuint),
                 element_data, hint);

    //--------------------------------------------------------------------------
    // Shader attribute pointers
    //--------------------------------------------------------------------------
    // TODO: How to get size of mesh_vertex.pos dynamically? Why doesn't
    //       sizeof_member work?
    glVertexAttribPointer(RICO_SHADER_POS_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct mesh_vertex),
                          (GLvoid *)offsetof(struct mesh_vertex, pos));
    glEnableVertexAttribArray(RICO_SHADER_POS_LOC);

    glVertexAttribPointer(RICO_SHADER_NORMAL_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct mesh_vertex),
                          (GLvoid *)offsetof(struct mesh_vertex, normal));
    glEnableVertexAttribArray(RICO_SHADER_NORMAL_LOC);

    glVertexAttribPointer(RICO_SHADER_COL_LOC, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct mesh_vertex),
                          (GLvoid *)offsetof(struct mesh_vertex, col));
    glEnableVertexAttribArray(RICO_SHADER_COL_LOC);

    glVertexAttribPointer(RICO_SHADER_UV_LOC, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct mesh_vertex),
                          (GLvoid *)offsetof(struct mesh_vertex, uv));
    glEnableVertexAttribArray(RICO_SHADER_UV_LOC);

    // Clean up
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return SUCCESS;
}

int mesh_free(struct rico_mesh *mesh)
{
    // HACK: For now, don't ever delete meshes loaded from *.ric files.
    //       Eventually, there should be an unload_obj_file method or some
    //       other mesh cleanup process.
    if (mesh->type == MESH_OBJ_WORLD)
    {
#if RICO_DEBUG_MESH
        printf("[mesh][WARN] uid=%d name=%s Disabled .RIC mesh free.\n",
               mesh->hnd.uid, mesh->hnd.name);
#endif
        return SUCCESS;
    }

#if RICO_DEBUG_MESH
    printf("[mesh][free] uid=%d name=%s\n", mesh->hnd.uid, mesh->hnd.name);
#endif

    hashtable_delete_hnd(&global_meshes, &mesh->hnd);

    glDeleteBuffers(2, mesh->vbos);
    glDeleteVertexArrays(1, &mesh->vao);
    return pool_remove(mesh->hnd.pool, mesh->hnd.id);
}

void mesh_update(struct rico_mesh *mesh)
{
    UNUSED(mesh);
    //TODO: Animate the mesh.
}

void mesh_render(struct rico_mesh *mesh)
{
    // Draw
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
