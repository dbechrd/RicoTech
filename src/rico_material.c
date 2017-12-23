global const char *material_name(struct rico_material *material)
{
    return (u8 *)material + material->name_offset;
}

void material_bind(struct rico_material *material)
{
#if RICO_DEBUG_MATERIAL
    printf("[ mtl][bind] name=%s\n", material_name(material));
#endif

    struct rico_texture *tex_diffuse;
    struct rico_texture *tex_specular;

    tex_diffuse = pack_read(pack_active, material->tex_diffuse_id);
    tex_specular = pack_read(pack_active, material->tex_specular_id);

    texture_bind(tex_diffuse, GL_TEXTURE0);
    texture_bind(tex_specular, GL_TEXTURE1);
}

void material_unbind(struct rico_material *material)
{
#if RICO_DEBUG_MATERIAL
    printf("[ mtl][unbd] name=%s\n", material_name(material));
#endif

    struct rico_texture *tex_diffuse;
    struct rico_texture *tex_specular;

    tex_diffuse = pack_read(pack_active, material->tex_diffuse_id);
    tex_specular = pack_read(pack_active, material->tex_specular_id);

    texture_unbind(tex_diffuse, GL_TEXTURE0);
    texture_unbind(tex_specular, GL_TEXTURE1);
}