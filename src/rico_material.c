const u32 RICO_MATERIAL_SIZE = sizeof(struct rico_material);

u32 RICO_DEFAULT_MATERIAL = 0;

internal inline struct rico_pool **material_pool_ptr()
{
    struct rico_chunk *chunk = chunk_active();
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->materials);
    return &chunk->materials;
}

internal inline struct rico_pool *material_pool()
{
    return *material_pool_ptr();
}

internal inline struct rico_material *material_find(u32 handle)
{
    struct rico_material *material = pool_read(material_pool(), handle);
    RICO_ASSERT(material);
    return material;
}

// TODO: Do proper reference counting, this function is stupid.
u32 material_request(u32 handle)
{
    struct rico_material *material = material_find(handle);
    material->ref_count++;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][rqst] uid=%d ref=%d name=%s\n", material->uid.uid,
           material->ref_count, material->uid.name);
#endif

    return handle;
}

int material_request_by_name(u32 *_handle, const char *name)
{
    u32 handle = hashtable_search_by_name(&global_materials, name);
    if (!handle)
    {
        return RICO_ERROR(ERR_MATERIAL_INVALID_NAME, "Material not found: %s.",
                          name);
    }

    *_handle = material_request(handle);
    return SUCCESS;
}

int material_init(u32 *_handle, const char *name, u32 tex_diffuse,
                  u32 tex_specular, float shiny)
{
#if RICO_DEBUG_MATERIAL
    printf("[ mtl][init] name=%s\n", name);
#endif

    enum rico_error err;
    *_handle = RICO_DEFAULT_MATERIAL;

    u32 handle;
    err = pool_handle_alloc(material_pool_ptr(), &handle);
    if (err) return err;

    struct rico_material *material = material_find(handle);
    uid_init(&material->uid, RICO_UID_MATERIAL, name, true);
    material->tex_diffuse = texture_request(tex_diffuse);
    material->tex_specular = texture_request(tex_specular);
    material->shiny = shiny;

    // Store in global hash table
    hash_key key = hashgen_str(material->uid.name);
    err = hashtable_insert(&global_materials, key, handle);
    if (err) return err;

    *_handle = handle;
    return err;
}

void material_free(u32 handle)
{
    struct rico_material *material = material_find(handle);
    if (material->ref_count > 0)
        material->ref_count--;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][ rls] uid=%d ref=%d name=%s\n", material->uid.uid,
           material->ref_count, material->uid.name);
#endif

    if (material->ref_count > 0)
        return;

    // TODO: Use fixed pool slots
    if (handle == RICO_DEFAULT_MATERIAL)
        return;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][free] uid=%d name=%s\n", material->uid.uid,
           material->uid.name);
#endif

    hash_key key = hashgen_str(material->uid.name);
    hashtable_delete(&global_materials, key);

    texture_free(material->tex_diffuse);
    texture_free(material->tex_specular);

    material->uid.uid = UID_NULL;
    pool_handle_free(material_pool(), handle);
}

inline const char *material_name(u32 handle)
{
    struct rico_material *material = material_find(handle);
    return material->uid.name;
}

inline float material_shiny(u32 handle)
{
    struct rico_material *material = material_find(handle);
    return material->shiny;
}

void material_bind(u32 handle)
{
    struct rico_material *material = material_find(handle);
    texture_bind(material->tex_diffuse, GL_TEXTURE0);
    texture_bind(material->tex_specular, GL_TEXTURE1);
}

void material_unbind(u32 handle)
{
    struct rico_material *material = material_find(handle);
    texture_unbind(material->tex_diffuse, GL_TEXTURE0);
    texture_unbind(material->tex_specular, GL_TEXTURE1);
}

//int material_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(material_serialize_0)
{
    const struct rico_material *mat = handle;

    fwrite(&mat->ref_count,    sizeof(mat->ref_count),    1, file->fs);
    fwrite(&mat->tex_diffuse,  sizeof(mat->tex_diffuse),  1, file->fs);
    fwrite(&mat->tex_specular, sizeof(mat->tex_specular), 1, file->fs);
    fwrite(&mat->shiny,        sizeof(mat->shiny),        1, file->fs);
    return SUCCESS;
}

//int material_deserialize_0(void *_handle, const struct rico_file *file)
DESERIAL(material_deserialize_0)
{
    u32 diffuse, specular;

    struct rico_material *mat = *_handle;
    fread(&mat->ref_count, sizeof(mat->ref_count),    1, file->fs);
    fread(&diffuse,        sizeof(mat->tex_diffuse),  1, file->fs);
    fread(&specular,       sizeof(mat->tex_specular), 1, file->fs);
    fread(&mat->shiny,     sizeof(mat->shiny),        1, file->fs);

    mat->tex_diffuse  = texture_request(diffuse);
    mat->tex_specular = texture_request(specular);
    return SUCCESS;
}
