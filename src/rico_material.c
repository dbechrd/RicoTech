struct pool_id RICO_DEFAULT_MATERIAL;

int material_init(struct rico_material *material, const char *name,
                  struct pool_id tex_diffuse_id, struct pool_id tex_specular_id,
                  float shiny)
{
    enum rico_error err;

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][init] name=%s\n", name);
#endif

    hnd_init(&material->hnd, RICO_HND_MATERIAL, name);
    material->tex_diffuse_id = chunk_dupe(material->hnd.chunk, tex_diffuse_id);
    material->tex_specular_id = chunk_dupe(material->hnd.chunk,
                                           tex_specular_id);
    material->shiny = shiny;

    // Store in global hash table
    err = hashtable_insert_hnd(&global_materials, &material->hnd, material);
    return err;
}

int material_free(struct rico_material *material)
{
#if RICO_DEBUG_MATERIAL
    printf("[ mtl][free] uid=%d name=%s\n", material->hnd.uid,
           material->hnd.name);
#endif

    hashtable_delete_hnd(&global_materials, &material->hnd);

    chunk_free(material->hnd.chunk, material->tex_diffuse_id);
    chunk_free(material->hnd.chunk, material->tex_specular_id);
    pool_remove(material->hnd.pool, material->hnd.id);
}

void material_bind(struct rico_material *material)
{
    struct rico_texture *tex_diffuse = chunk_read(material->hnd.chunk,
                                                  material->tex_diffuse_id);
    struct rico_texture *tex_specular = chunk_read(material->hnd.chunk,
                                                   material->tex_specular_id);
    texture_bind(tex_diffuse, GL_TEXTURE0);
    texture_bind(tex_specular, GL_TEXTURE1);
}

void material_unbind(struct rico_material *material)
{
    struct rico_texture *tex_diffuse = chunk_read(material->hnd.chunk,
                                                  material->tex_diffuse_id);
    struct rico_texture *tex_specular = chunk_read(material->hnd.chunk,
                                                   material->tex_specular_id);
    texture_unbind(tex_diffuse, GL_TEXTURE0);
    texture_unbind(tex_specular, GL_TEXTURE1);
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
