const u32 RICO_MESH_SIZE = sizeof(struct rico_mesh);

const char *rico_mesh_type_string[] = {
    RICO_MESH_TYPES(GEN_STRING)
};

struct hnd RICO_DEFAULT_MESH = { 0 };

internal inline struct rico_pool **mesh_pool_ptr(enum rico_persist persist)
{
    struct rico_chunk *chunk = chunk_active();
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->pools[persist][POOL_MESHES]);
    return &chunk->pools[persist][POOL_MESHES];
}

internal inline struct rico_pool *mesh_pool(enum rico_persist persist)
{
    return *mesh_pool_ptr(persist);
}

internal inline struct rico_mesh *mesh_find(struct hnd handle)
{
    struct rico_mesh *mesh = pool_read(mesh_pool(handle.persist), handle.value);
    RICO_ASSERT(mesh);
    return mesh;
}

internal int build_mesh(struct rico_mesh *mesh, u32 vertex_count,
                        const struct mesh_vertex *vertex_data,
                        u32 element_count, const GLuint *element_data,
                        GLenum hint);

struct hnd mesh_request(struct hnd handle)
{
    struct rico_mesh *mesh = mesh_find(handle);
    mesh->ref_count++;

#if RICO_DEBUG_MESH
    printf("[mesh][rqst] uid=%d ref=%d name=%s\n", mesh->uid.uid,
           mesh->ref_count, mesh->uid.name);
#endif

    return handle;
}

int mesh_request_by_name(struct hnd *_handle, enum rico_persist persist,
                         const char *name)
{
    struct hnd handle = hashtable_search_by_name(&global_meshes, name);
    if (!handle.value)
    {
        return RICO_ERROR(ERR_MESH_INVALID_NAME, "Mesh not found: %s", name);
    }

    *_handle = mesh_request(handle);
    return SUCCESS;

    // Cleanup: Pre hash table lookup
#if 0
    u32 first = pool_handle_first(mesh_pool());
    if (!first) return 0;

    struct handle handle = first;
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
#endif
}

enum rico_mesh_type mesh_type_get(struct hnd handle)
{
    if (!handle.value)
        return MESH_NULL;

    struct rico_mesh *mesh = mesh_find(handle);
    return mesh->type;
}

bool mesh_selectable(struct hnd handle)
{
    enum rico_mesh_type type = mesh_type_get(handle);
    return (type != MESH_NULL &&
            type != MESH_STRING_SCREEN);
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

int mesh_load(struct hnd *_handle, enum rico_persist persist, const char *name,
              enum rico_mesh_type type, u32 vertex_count,
              const struct mesh_vertex *vertex_data, u32 element_count,
              const GLuint *element_data, GLenum hint)
{
#if RICO_DEBUG_MESH
    printf("[mesh][init] name=%s vertices=%d\n", name, vertex_count);
#endif

    enum rico_error err;

    struct hnd handle;
    err = pool_handle_alloc(mesh_pool_ptr(persist), &handle);
    if (err) return err;

    struct rico_mesh *mesh = mesh_find(handle);

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

const char *mesh_name(struct hnd handle)
{
    struct rico_mesh *mesh = mesh_find(handle);
    return mesh->uid.name;
}

const struct bbox *mesh_bbox(struct hnd handle)
{
    struct rico_mesh *mesh = mesh_find(handle);
    return &mesh->bbox;
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