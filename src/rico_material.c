const u32 RICO_MATERIAL_SIZE = sizeof(struct rico_material);

global struct rico_material *RICO_DEFAULT_MATERIAL;

struct rico_material *material_request(struct rico_material *material)
{
    // TODO: Do proper reference counting, this function is stupid.
    RICO_ASSERT(0);
    material->ref_count++;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][rqst] uid=%d ref=%d name=%s\n", material->hnd.uid,
           material->ref_count, material->hnd.name);
#endif

    return material;
}

int material_init(struct rico_material *material, const char *name,
                  struct rico_texture *tex_diffuse,
                  struct rico_texture *tex_specular, float shiny)
{
    enum rico_error err;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][init] name=%s\n", name);
#endif

    hnd_init(&material->hnd, RICO_HND_MATERIAL, name);
    material->tex_diffuse = tex_diffuse;
    tex_diffuse->ref_count++;
    material->tex_specular = tex_specular;
    tex_specular->ref_count++;
    material->shiny = shiny;

    // Store in global hash table
    err = hashtable_insert(&global_materials, material->hnd.name,
                           material->hnd.len, material);

    return err;
}

void material_free(struct rico_material *material)
{
    if (material->ref_count > 0)
        material->ref_count--;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][ rls] uid=%d ref=%d name=%s\n", material->hnd.uid,
           material->ref_count, material->hnd.name);
#endif

    if (material->ref_count > 0)
        return;

    // Cleanup: Use fixed pool slots
    //if (handle == RICO_DEFAULT_MATERIAL)
    //    return;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][free] uid=%d name=%s\n", material->hnd.uid,
           material->hnd.name);
#endif

    hashtable_delete(&global_materials, material->hnd.name, material->hnd.len);

    texture_free(material->tex_diffuse);
    texture_free(material->tex_specular);

    material->hnd.uid = UID_NULL;
    struct rico_pool *pool = chunk_pool(chunk_active, RICO_HND_MATERIAL);
    pool_handle_free(pool, &material->hnd);
}

// TODO: Deprecate pointless accessors like this
internal inline const char *material_name(struct rico_material *material)
{
    return material->hnd.name;
}

// TODO: Deprecate pointless accessors like this
internal inline float material_shiny(struct rico_material *material)
{
    return material->shiny;
}

void material_bind(struct rico_material *material)
{
    texture_bind_diff(material->tex_diffuse);
    texture_bind_spec(material->tex_specular);
}

void material_unbind(struct rico_material *material)
{
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
