struct rico_material *RICO_DEFAULT_MATERIAL;

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
    err = hashtable_insert_hnd(&global_materials, &material->hnd, material);
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

    hashtable_delete_hnd(&global_materials, &material->hnd);

    texture_free(material->tex_diffuse);
    texture_free(material->tex_specular);

    material->hnd.uid = UID_NULL;
    chunk_free(chunk_active, &material->hnd);
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
    if (material->tex_diffuse)
    {
        texture_bind(material->tex_diffuse, GL_TEXTURE0);
    }
    else
    {
#if RICO_DEBUG_MATERIAL
        printf("[ mtl][warn] uid=%d name=%s No diffuse texture, using default\n", material->hnd.uid,
               material->hnd.name);
        texture_bind(RICO_DEFAULT_TEXTURE_DIFF, GL_TEXTURE0);
#endif
    }

    if (material->tex_specular)
    {
        texture_bind(material->tex_specular, GL_TEXTURE1);
    }
    else
    {
#if RICO_DEBUG_MATERIAL
        printf("[ mtl][warn] uid=%d name=%s No specular texture, using default\n", material->hnd.uid,
               material->hnd.name);
        texture_bind(RICO_DEFAULT_TEXTURE_SPEC, GL_TEXTURE1);
#endif
    }
}

void material_unbind(struct rico_material *material)
{
    texture_unbind(material->tex_diffuse, GL_TEXTURE0);
    texture_unbind(material->tex_specular, GL_TEXTURE1);
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
