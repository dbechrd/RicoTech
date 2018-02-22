static void material_bind(pkid pkid)
{
    struct rico_material *material = RICO_pack_lookup(pkid);

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][bind] name=%s\n", material_name(material));
#endif

    // Bind diffuse or default
    if (material->tex_albedo)
    {
        texture_bind(material->tex_albedo, GL_TEXTURE0);
    }
    else
    {
        texture_bind(TEXTURE_DEFAULT_DIFF, GL_TEXTURE0);
    }

    // Bind specular or default
    if (material->tex_mrao)
    {
        texture_bind(material->tex_mrao, GL_TEXTURE1);
    }
    else
    {
        texture_bind(TEXTURE_DEFAULT_SPEC, GL_TEXTURE1);
    }

    // Bind emission or default
    if (material->tex_emission)
    {
        texture_bind(material->tex_emission, GL_TEXTURE2);
    }
    else
    {
        texture_bind(TEXTURE_DEFAULT_EMIS, GL_TEXTURE2);
    }
}

static void material_unbind(pkid pkid)
{
    struct rico_material *material = RICO_pack_lookup(pkid);

#if RICO_DEBUG_MATERIAL
    printf("[ mtl][unbd] name=%s\n", material_name(material));
#endif

    // Unbind diffuse or default
    if (material->tex_albedo)
    {
        texture_unbind(material->tex_albedo, GL_TEXTURE0);
    }
    else
    {
        texture_unbind(TEXTURE_DEFAULT_DIFF, GL_TEXTURE0);
    }

    // Unbind specular or default
    if (material->tex_mrao)
    {
        texture_unbind(material->tex_mrao, GL_TEXTURE1);
    }
    else
    {
        texture_unbind(TEXTURE_DEFAULT_SPEC, GL_TEXTURE1);
    }

    // Unbind emission or default
    if (material->tex_emission)
    {
        texture_unbind(material->tex_emission, GL_TEXTURE2);
    }
    else
    {
        texture_unbind(TEXTURE_DEFAULT_EMIS, GL_TEXTURE2);
    }
}