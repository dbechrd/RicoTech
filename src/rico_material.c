const u32 RICO_MATERIAL_SIZE = sizeof(struct rico_material);

u32 RICO_DEFAULT_MATERIAL = 0;

internal inline struct rico_pool **material_pool_ptr(enum rico_persist persist)
{
    struct rico_chunk *chunk = chunk_active();
    RICO_ASSERT(chunk);
    RICO_ASSERT(chunk->pools[POOL_ITEMTYPE_MATERIALS][persist]);
    return &chunk->pools[POOL_ITEMTYPE_MATERIALS][persist];
}

internal inline struct rico_pool *material_pool(enum rico_persist persist)
{
    return *material_pool_ptr(persist);
}

internal inline struct rico_material *material_find(enum rico_persist persist,
                                                    u32 handle)
{
    struct rico_material *material = pool_read(material_pool(persist), handle);
    RICO_ASSERT(material);
    return material;
}

// TODO: Do proper reference counting, this function is stupid.
u32 material_request(enum rico_persist persist, u32 handle)
{
    struct rico_material *material = material_find(persist, handle);
    material->ref_count++;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][rqst] uid=%d ref=%d name=%s\n", material->uid.uid,
           material->ref_count, material->uid.name);
#endif

    return handle;
}

int material_request_by_name(u32 *_handle, enum rico_persist persist,
                             const char *name)
{
    u32 handle = hashtable_search_by_name(&global_materials, name);
    if (!handle)
    {
        return RICO_ERROR(ERR_MATERIAL_INVALID_NAME, "Material not found: %s.",
                          name);
    }

    *_handle = material_request(persist, handle);
    return SUCCESS;
}

int material_init(u32 *_handle, enum rico_persist persist, const char *name,
                  u32 tex_diffuse, u32 tex_specular, float shiny)
{
#if RICO_DEBUG_MATERIAL
    printf("[ mtl][init] name=%s\n", name);
#endif

    enum rico_error err;
    *_handle = RICO_DEFAULT_MATERIAL;

    u32 handle;
    err = pool_handle_alloc(material_pool_ptr(persist), &handle);
    if (err) return err;

    struct rico_material *material = material_find(persist, handle);
    uid_init(&material->uid, RICO_UID_MATERIAL, name, true);
    material->tex_diffuse = texture_request(persist, tex_diffuse);
    material->tex_specular = texture_request(persist, tex_specular);
    material->shiny = shiny;

    // Store in global hash table
    hash_key key = hashgen_str(material->uid.name);
    err = hashtable_insert(&global_materials, key, handle);
    if (err) return err;

    *_handle = handle;
    return err;
}

void material_free(enum rico_persist persist, u32 handle)
{
    struct rico_material *material = material_find(persist, handle);
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

    texture_free(persist, material->tex_diffuse);
    texture_free(persist, material->tex_specular);

    material->uid.uid = UID_NULL;
    pool_handle_free(material_pool(persist), handle);
}

inline const char *material_name(enum rico_persist persist, u32 handle)
{
    struct rico_material *material = material_find(persist, handle);
    return material->uid.name;
}

inline float material_shiny(enum rico_persist persist, u32 handle)
{
    struct rico_material *material = material_find(persist, handle);
    return material->shiny;
}

void material_bind(enum rico_persist persist, u32 handle)
{
    struct rico_material *material = material_find(persist, handle);
    texture_bind(persist, material->tex_diffuse, GL_TEXTURE0);
    texture_bind(persist, material->tex_specular, GL_TEXTURE1);
}

void material_unbind(enum rico_persist persist, u32 handle)
{
    struct rico_material *material = material_find(persist, handle);
    texture_unbind(persist, material->tex_diffuse, GL_TEXTURE0);
    texture_unbind(persist, material->tex_specular, GL_TEXTURE1);
}

#if 0
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
#endif