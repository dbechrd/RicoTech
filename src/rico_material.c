global const char *material_name(struct rico_material *material)
{
    return (u8 *)material + material->name_offset;
}

void material_bind(struct pack *pack, u32 id, GLint shiny_loc)
{
    RICO_ASSERT(pack);
    RICO_ASSERT(id < pack->blobs_used);

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][bind] name=%s\n", material_name(material));
#endif
    
    struct rico_material *material = pack_read(pack, id);
    glUniform1f(shiny_loc, material->shiny);

    // Bind diffuse or default
    if (material->tex_diffuse_id)
    {
        texture_bind(pack, material->tex_diffuse_id, GL_TEXTURE0);
    }
    else
    {
        texture_bind(pack_default, TEXTURE_DEFAULT_DIFF, GL_TEXTURE0);
    }

    // Bind specular or default
    if (material->tex_specular_id)
    {
        texture_bind(pack, material->tex_specular_id, GL_TEXTURE1);
    }
    else
    {
        texture_bind(pack_default, TEXTURE_DEFAULT_SPEC, GL_TEXTURE1);
    }
}

void material_unbind(struct pack *pack, u32 id)
{
    RICO_ASSERT(pack);
    RICO_ASSERT(id < pack->blobs_used);

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][unbd] name=%s\n", material_name(material));
#endif

    struct rico_material *material = pack_read(pack, id);

    // Bind diffuse or default
    if (material->tex_diffuse_id)
    {
        texture_unbind(pack, material->tex_diffuse_id, GL_TEXTURE0);
    }
    else
    {
        texture_unbind(pack_default, TEXTURE_DEFAULT_DIFF, GL_TEXTURE0);
    }

    // Bind specular or default
    if (material->tex_specular_id)
    {
        texture_unbind(pack, material->tex_specular_id, GL_TEXTURE1);
    }
    else
    {
        texture_unbind(pack_default, TEXTURE_DEFAULT_SPEC, GL_TEXTURE1);
    }
}