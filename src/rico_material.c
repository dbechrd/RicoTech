const u32 RICO_MATERIAL_SIZE = sizeof(struct rico_material);

struct hnd RICO_DEFAULT_MATERIAL = { 0 };

internal inline struct rico_material *material_find(struct hnd handle)
{
    struct rico_chunk *chunk = chunk_active();
    struct rico_material *material = pool_read(chunk_pool(chunk, handle.persist, POOL_MATERIALS),
                                               handle.value);
    RICO_ASSERT(material->uid.uid);
    return material;
}

// TODO: Do proper reference counting, this function is stupid.
struct hnd material_request(struct hnd handle)
{
    struct rico_material *material = material_find(handle);
    material->ref_count++;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][rqst] uid=%d ref=%d name=%s\n", material->uid.uid,
           material->ref_count, material->uid.name);
#endif

    return handle;
}

int material_request_by_name(struct hnd *_handle, const char *name)
{
    struct hnd handle = hashtable_search_by_name(&global_materials, name);
    if (!handle.value)
    {
        return RICO_ERROR(ERR_MATERIAL_INVALID_NAME, "Material not found: %s.",
                          name);
    }

    *_handle = material_request(handle);
    return SUCCESS;
}

int material_init(struct hnd *_handle, enum rico_persist persist,
                  const char *name, struct hnd tex_diffuse,
                  struct hnd tex_specular, float shiny)
{
    enum rico_error err;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][init] name=%s\n", name);
#endif

    struct hnd handle;
    struct rico_material *material;
    err = pool_handle_alloc(material_pool_ptr(persist), &handle, (void *)&material);
    if (err) return err;

    uid_init(&material->uid, RICO_UID_MATERIAL, name, true);
    material->tex_diffuse = texture_request(tex_diffuse);
    material->tex_specular = texture_request(tex_specular);
    material->shiny = shiny;

    // Store in global hash table
    hash_key key = hashgen_str(material->uid.name);
    err = hashtable_insert(&global_materials, key, handle);
    if (err) return err;

    if(_handle) *_handle = handle;
    return err;
}

void material_free(struct hnd handle)
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

    // Cleanup: Use fixed pool slots
    //if (handle == RICO_DEFAULT_MATERIAL)
    //    return;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][free] uid=%d name=%s\n", material->uid.uid,
           material->uid.name);
#endif

    hash_key key = hashgen_str(material->uid.name);
    hashtable_delete(&global_materials, key);

    texture_free(material->tex_diffuse);
    texture_free(material->tex_specular);

    material->uid.uid = UID_NULL;
    pool_handle_free(material_pool(handle.persist), handle);
}

internal inline const char *material_name(struct hnd handle)
{
    struct rico_material *material = material_find(handle);
    return material->uid.name;
}

internal inline float material_shiny(struct hnd handle)
{
    struct rico_material *material = material_find(handle);
    return material->shiny;
}

void material_bind(struct hnd handle)
{
    struct rico_material *material = material_find(handle);
    RICO_ASSERT(material->uid.uid != UID_NULL);

    texture_bind_diff(material->tex_diffuse);
    texture_bind_spec(material->tex_specular);
}

void material_unbind(struct hnd handle)
{
    struct rico_material *material = material_find(handle);
    RICO_ASSERT(material->uid.uid != UID_NULL);

    texture_unbind_diff(material->tex_diffuse);
    texture_unbind_spec(material->tex_specular);
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
