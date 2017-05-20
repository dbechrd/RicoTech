const u32 RICO_MESH_SIZE = sizeof(struct rico_mesh);

const char *rico_mesh_type_string[] = {
    RICO_MESH_TYPES(GEN_STRING)
};

u32 RICO_DEFAULT_MESH = 0;

internal inline struct rico_pool **mesh_pool_ptr()
{
    struct rico_chunk *chunk = chunk_active();
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->meshes);
    return &chunk->meshes;
}

internal inline struct rico_pool *mesh_pool()
{
    return *mesh_pool_ptr();
}

internal inline struct rico_mesh *mesh_find(u32 handle)
{
    struct rico_mesh *mesh = pool_read(mesh_pool(), handle);
    RICO_ASSERT(mesh);
    return mesh;
}

internal int build_mesh(struct rico_mesh *mesh, u32 vertex_count,
                      const struct mesh_vertex *vertex_data,
                      u32 element_count, const GLuint *element_data,
                      GLenum hint);

//TODO: Should be requesting const char* filename or u32 filename (string hash)
u32 mesh_request(u32 handle)
{
    struct rico_mesh *mesh = mesh_find(handle);
    mesh->ref_count++;

#if RICO_DEBUG_MESH
    printf("[mesh][rqst] uid=%d ref=%d name=%s\n", mesh->uid.uid,
           mesh->ref_count, mesh->uid.name);
#endif

    return handle;
}

u32 mesh_request_by_name(const char *name)
{
    u32 first = pool_handle_first(mesh_pool());
    if (!first) return 0;

    u32 handle = first;
    while (1)
    {
        // Found it!
        if (strcmp(mesh_name(handle), name) == 0)
            break;

        // Keep looking (unless we're back at the start)
        handle = pool_handle_next(mesh_pool(), handle);
        if (handle == first)
            return 0;
    }

    return mesh_request(handle);
}

enum rico_mesh_type mesh_type_get(u32 handle)
{
    if (!handle)
        return MESH_NULL;

    struct rico_mesh *mesh = mesh_find(handle);
    return mesh->type;
}

bool mesh_selectable(u32 handle)
{
    enum rico_mesh_type type = mesh_type_get(handle);
    return (type != MESH_NULL &&
            type != MESH_STRING_SCREEN);
}

u32 mesh_next(u32 handle)
{
    u32 start = pool_handle_next(mesh_pool(), handle);
    u32 next = start;

    do
    {
        if (mesh_selectable(next))
            return next;

        next = pool_handle_next(mesh_pool(), next);
    } while (next != start);

    return 0;
}

u32 mesh_prev(u32 handle)
{
    u32 start = pool_handle_prev(mesh_pool(), handle);
    u32 prev = start;

    do
    {
        if (mesh_selectable(prev))
            return prev;

        prev = pool_handle_prev(mesh_pool(), prev);
    } while (prev != start);

    return 0;
}

int mesh_load(u32 *_handle, const char *name, enum rico_mesh_type type,
              u32 vertex_count, const struct mesh_vertex *vertex_data,
              u32 element_count, const GLuint *element_data, GLenum hint)
{
#if RICO_DEBUG_MESH
    printf("[mesh][init] name=%s vertices=%d\n", name, vertex_count);
#endif

    enum rico_error err;
    *_handle = RICO_DEFAULT_MESH;

    err = pool_handle_alloc(mesh_pool_ptr(), _handle);
    if (err) return err;

    struct rico_mesh *mesh = mesh_find(*_handle);

    // Note: If we want to serialize mesh data we have to store the vertex data
    //       and element array in the struct.
    uid_init(&mesh->uid, RICO_UID_MESH, name, false);
    mesh->type = type;

    err = build_mesh(mesh, vertex_count, vertex_data, element_count,
                     element_data, hint);
    if (err) return err;

    err = bbox_init_mesh(&mesh->bbox, name, vertex_data, vertex_count,
                         COLOR_WHITE_HIGHLIGHT);
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

void mesh_free(u32 handle)
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

    // TODO: Use fixed pool slots
    if (handle == RICO_DEFAULT_MESH)
        return;

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

    bbox_free_mesh(&mesh->bbox);

    glDeleteBuffers(2, mesh->vbos);
    glDeleteVertexArrays(1, &mesh->vao);

    mesh->uid.uid = UID_NULL;
    pool_handle_free(mesh_pool(), handle);
}

const char *mesh_name(u32 handle)
{
    struct rico_mesh *mesh = mesh_find(handle);
    return mesh->uid.name;
}

const struct bbox *mesh_bbox(u32 handle)
{
    struct rico_mesh *mesh = mesh_find(handle);
    return &mesh->bbox;
}

void mesh_update(u32 handle)
{
    UNUSED(handle);
    //TODO: Animate the mesh.
}

void mesh_render(u32 handle)
{
    struct rico_mesh *mesh = mesh_find(handle);

    // Draw
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}