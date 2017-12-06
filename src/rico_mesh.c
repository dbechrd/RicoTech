const u32 RICO_MESH_SIZE = sizeof(struct rico_mesh);

const char *rico_mesh_type_string[] = {
    RICO_MESH_TYPES(GEN_STRING)
};

struct hnd RICO_DEFAULT_MESH = { 0 };

internal int build_mesh(struct rico_mesh *mesh, u32 vertex_count,
                        const struct mesh_vertex *vertex_data,
                        u32 element_count, const GLuint *element_data,
                        GLenum hint);

bool mesh_selectable(struct hnd handle)
{
    return (mesh->type != MESH_STRING_SCREEN);
}

struct hnd mesh_next(struct hnd handle)
{
    struct hnd start = pool_handle_next(mesh_pool(handle.persist), handle);
    struct hnd next = start;

    do
    {
        if (mesh_selectable(next))
            return next;

        next = pool_handle_next(mesh_pool(handle.persist), next);
    } while (next.value != start.value);

    return HANDLE_NULL;
}

struct hnd mesh_prev(struct hnd handle)
{
    struct hnd start = pool_handle_prev(mesh_pool(handle.persist), handle);
    struct hnd prev = start;

    do
    {
        if (mesh_selectable(prev))
            return prev;

        prev = pool_handle_prev(mesh_pool(handle.persist), prev);
    } while (prev.value != start.value);

    return HANDLE_NULL;
}

int mesh_init(struct rico_mesh *_mesh, const char *name,
              enum rico_mesh_type type, u32 vertex_count,
              const struct mesh_vertex *vertex_data, u32 element_count,
              const GLuint *element_data, GLenum hint);
{
#if RICO_DEBUG_MESH
    printf("[mesh][init] name=%s vertices=%d\n", name, vertex_count);
#endif

    enum rico_error err;

    struct hnd handle;
    struct rico_mesh *mesh;
    err = pool_handle_alloc(mesh_pool_ptr(persist), &handle, (void *)&mesh);
    if (err) return err;

    // Note: If we want to serialize mesh data we have to store the vertex data
    //       and element array in the struct.
    uid_init(&mesh->uid, RICO_UID_MESH, name, false);
    mesh->type = type;

    err = build_mesh(mesh, vertex_count, vertex_data, element_count,
                     element_data, hint);
    if (err) return err;

    err = bbox_init_mesh(&mesh->bbox, name, vertex_data, vertex_count,
                         COLOR_WHITE_HIGHLIGHT);
    if (err) return err;

    // Store in global hash table
    hash_key key = hashgen_str(mesh->uid.name);
    err = hashtable_insert(&global_meshes, key, handle);
    if (err) return err;

    if (_handle) *_handle = handle;
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

void mesh_free(struct hnd handle)
{
    struct rico_mesh *mesh = mesh_find(handle);
    if (mesh->ref_count > 0)
        mesh->ref_count--;

#if RICO_DEBUG_MESH
    printf("[mesh][ rls] uid=%d ref=%d name=%s\n", mesh->uid.uid,
           mesh->ref_count, mesh->uid.name);
#endif

    if (mesh->ref_count > 0)
        return;

    // TODO: Use fixed pool slots or request and never release at initialize
    //if (handle == RICO_DEFAULT_MESH)
    //    return;

    // HACK: For now, don't ever delete meshes loaded from *.ric files.
    //       Eventually, there should be an unload_obj_file method or some
    //       other mesh cleanup process.
    if (mesh->type == MESH_OBJ_WORLD)
    {
#if RICO_DEBUG_MESH
        printf("[mesh][WARN] uid=%d name=%s Disabled .RIC mesh free.\n",
               mesh->uid.uid, mesh->uid.name);
#endif
        return;
    }

#if RICO_DEBUG_MESH
    printf("[mesh][free] uid=%d name=%s\n", mesh->uid.uid, mesh->uid.name);
#endif

    hash_key key = hashgen_str(mesh->uid.name);
    hashtable_delete(&global_meshes, key);

    //bbox_free_mesh(&mesh->bbox);

    glDeleteBuffers(2, mesh->vbos);
    glDeleteVertexArrays(1, &mesh->vao);

    mesh->uid.uid = UID_NULL;
    pool_handle_free(mesh_pool(handle.persist), handle);
}

void mesh_update(struct hnd handle)
{
    UNUSED(handle);
    //TODO: Animate the mesh.
}

void mesh_render(struct hnd handle)
{
    struct rico_mesh *mesh = mesh_find(handle);

    // Draw
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
