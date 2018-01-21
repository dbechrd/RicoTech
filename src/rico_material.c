global const char *material_name(struct rico_material *material)
{
    RICO_ASSERT(material->name_offset);
    return (char *)((u8 *)material + material->name_offset);
}

void material_bind(struct pack *pack, u32 id)
{
    RICO_ASSERT(pack);
    struct rico_material *material = pack_lookup(pack, id);

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][bind] name=%s\n", material_name(material));
#endif

    // Bind diffuse or default
    if (material->tex_id[0])
    {
        texture_bind(pack, material->tex_id[0], GL_TEXTURE0);
    }
    else
    {
        texture_bind(pack_default, TEXTURE_DEFAULT_DIFF, GL_TEXTURE0);
    }

    // Bind specular or default
    if (material->tex_id[1])
    {
        texture_bind(pack, material->tex_id[1], GL_TEXTURE1);
    }
    else
    {
        texture_bind(pack_default, TEXTURE_DEFAULT_SPEC, GL_TEXTURE1);
    }

    // Bind emission or default
    if (material->tex_id[2])
    {
        texture_bind(pack, material->tex_id[2], GL_TEXTURE2);
    }
    else
    {
        texture_bind(pack_default, TEXTURE_DEFAULT_EMIS, GL_TEXTURE2);
    }
}

void material_unbind(struct pack *pack, u32 id)
{
    RICO_ASSERT(pack);
    struct rico_material *material = pack_lookup(pack, id);

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][unbd] name=%s\n", material_name(material));
#endif

    // Unbind diffuse or default
    if (material->tex_id[0])
    {
        texture_unbind(pack, material->tex_id[0], GL_TEXTURE0);
    }
    else
    {
        texture_unbind(pack_default, TEXTURE_DEFAULT_DIFF, GL_TEXTURE0);
    }

    // Unbind specular or default
    if (material->tex_id[1])
    {
        texture_unbind(pack, material->tex_id[1], GL_TEXTURE1);
    }
    else
    {
        texture_unbind(pack_default, TEXTURE_DEFAULT_SPEC, GL_TEXTURE1);
    }

    // Unbind emission or default
    if (material->tex_id[2])
    {
        texture_unbind(pack, material->tex_id[2], GL_TEXTURE2);
    }
    else
    {
        texture_unbind(pack_default, TEXTURE_DEFAULT_EMIS, GL_TEXTURE2);
    }
}