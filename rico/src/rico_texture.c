internal u8 *texture_pixels(struct rico_texture *tex)
{
    return (u8 *)tex + tex->pixels_offset;
}

internal int texture_upload(struct rico_texture *texture)
{
    enum rico_error err = SUCCESS;

#if RICO_DEBUG_TEXTURE
    printf("[ tex][upld] name=%s\n", texture_name(texture));
#endif

    struct rgl_texture rgl_tex = { 0 };
    rgl_tex.gl_target = texture->gl_target;

    /*************************************************************************
    | Frequency of access:
    |
    | STREAM  Data store contents modified once and used at most a few times.
    | STATIC  Data store contents modified once and used many times.
    | DYNAMIC Data store contents modified repeatedly and used many times.
    |
    **************************************************************************
    | Nature of access:
    |
    | DRAW    The data store contents are modified by the application, and used
    |         as the source for GL drawing and image specification commands.
    | READ    The data store contents are modified by reading data from the GL,
    |         and used to return that data when queried by the application.
    | COPY    DRAW & READ
    |
    **************************************************************************
    | Common texture binding targets:
    |
    | GL_TEXTURE_1D         0.0f -> 1.0f (x)
    | GL_TEXTURE_2D         0.0f -> 1.0f (x, y)
    | GL_TEXTURE_3D         0.0f -> 1.0f (x, y, z)
    | GL_TEXTURE_RECTANGLE  0.0f -> width (x), 0.0f -> height (y)
    | GL_TEXTURE_CUBE_MAP   Six TEXTURE_2D where all width/height are equal.
    |
    **************************************************************************
    | Other targets:
    |
    | GL_TEXTURE_2D_MULTISAMPLE        Multiple samples (colors) per pixel.
    | GL_TEXTURE_1D_ARRAY              Alt: Multiple texture arrays.
    | GL_TEXTURE_2D_ARRAY               |
    | GL_TEXTURE_2D_MULTISAMPLE_ARRAY   |
    | GL_TEXTURE_CUBE_MAP_ARRAY         v
    |
    *************************************************************************/
    glCreateTextures(rgl_tex.gl_target, 1, &rgl_tex.gl_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(rgl_tex.gl_target, rgl_tex.gl_id);

    //--------------------------------------------------------------------------
    // Configure texture wrapping (for uv-coords outside range 0.0f - 1.0f)
    //--------------------------------------------------------------------------
    /*************************************************************************
    | Wrapping modes:
    |
    | GL_REPEAT            Repeat texture w/ same orientation.
    | GL_MIRRORED_REPEAT   Repeat texture mirrored over adjacted edges.
    | GL_CLAMP_TO_EDGE     Clamp to edge of geometry (stretch edges).
    | GL_CLAMP_TO_BORDER   Clamp to edge of texture (no stretching).
    |
    *************************************************************************/
    glTexParameteri(rgl_tex.gl_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(rgl_tex.gl_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(texture->gl_target, GL_TEXTURE_WRAP_R, GL_REPEAT);

    // For GL_CLAMP_TO_BORDER
    //float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    //glTexParameterfv(texture->gl_target, GL_TEXTURE_BORDER_COLOR, color);

    //--------------------------------------------------------------------------
    // Configure texture filtering
    //--------------------------------------------------------------------------
    /*************************************************************************
    | Filering modes:
    |
    | GL_NEAREST                Nearest pixel to coordinate.
    | GL_LINEAR                 Linear interpolation of nearest neighbors
    |                           (4 pixels in 2D).
    | GL_NEAREST_MIPMAP_NEAREST Nearest mipmap, nearest pixel.
    | GL_NEAREST_MIPMAP_LINEAR  Nearest mipmap, interpolate neighbor pixels.
    | GL_LINEAR_MIPMAP_NEAREST  Average 2 best mipmaps, nearest pixel.
    | GL_LINEAR_MIPMAP_LINEAR   Average 2 best mipmaps, interpolate neighbor
    |                           pixels.
    |
    *************************************************************************/
    glTexParameteri(rgl_tex.gl_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(rgl_tex.gl_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /*************************************************************************
    | GLenum    target
    | GLint     level
    | GLint     internalformat
    | GLsizei   width
    | GLsizei   height
    | GLint     border
    | GLenum    format
    | GLenum    type
    | const void *pixels
    *************************************************************************/
    // Tex creation params are dependent on BPP
    switch (texture->bpp)
    {
    case 8: // Greyscale (assume not gamma-corrected???)
        rgl_tex.format          = GL_RED;
        rgl_tex.format_internal = GL_RED;
        break;
    case 24: // RGB
        rgl_tex.format          = GL_RGB;
        // TODO: Store processed texture data in linear space and change this
        //       back to GL_RGB.
        rgl_tex.format_internal = GL_SRGB;
        break;
    case 32: // RGBA
        rgl_tex.format          = GL_RGBA;
        // TODO: Store processed texture data in linear space and change this
        //       back to GL_RGBA.
        rgl_tex.format_internal = GL_SRGB_ALPHA;
        break;
    default: // Unsupported BPP
        return RICO_ERROR(ERR_TEXTURE_UNSUPPORTED_BPP, NULL);
    }

    glTexImage2D(rgl_tex.gl_target, 0, rgl_tex.format_internal, texture->width,
                 texture->height, 0, rgl_tex.format, GL_UNSIGNED_BYTE,
                 texture_pixels(texture));

    //--------------------------------------------------------------------------
    // Generate mipmaps
    //--------------------------------------------------------------------------
    glGenerateMipmap(rgl_tex.gl_target);

    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    glBindTexture(rgl_tex.gl_target, 0);

    // Store in hash table
    hashtable_insert_pkid(&global_textures, texture->uid.pkid, &rgl_tex,
                         sizeof(rgl_tex));
    return err;
}

void texture_delete(struct rico_texture *texture)
{
    struct rgl_texture *rgl_tex =
        hashtable_search_pkid(&global_textures, texture->uid.pkid);
    if (!rgl_tex) return;

    glDeleteTextures(1, &rgl_tex->gl_id);

    hashtable_delete_pkid(&global_textures, texture->uid.pkid);
}

void texture_bind(pkid pkid, GLenum texture_unit)
{
    struct rgl_texture *rgl_tex = hashtable_search_pkid(&global_textures, pkid);
    if (!rgl_tex)
    {
        struct rico_texture *texture = pack_lookup(pkid);
        RICO_ASSERT(texture);
        texture_upload(texture);
        rgl_tex = hashtable_search_pkid(&global_textures, pkid);
    }
    RICO_ASSERT(rgl_tex);
    RICO_ASSERT(rgl_tex->gl_id);

#if RICO_DEBUG_TEXTURE
    struct rico_texture *texture = pack_lookup(pack, id);
    printf("[ tex][bind] name=%s\n", texture_name(texture));
#endif

    glActiveTexture(texture_unit);
    glBindTexture(rgl_tex->gl_target, rgl_tex->gl_id);
}

void texture_unbind(pkid pkid, GLenum texture_unit)
{
    struct rgl_texture *rgl_tex = hashtable_search_pkid(&global_textures, pkid);
    RICO_ASSERT(rgl_tex);

#if RICO_DEBUG_TEXTURE
    struct rico_texture *texture = pack_lookup(pack, uid->pkid);
    printf("[ tex][unbd] name=%s\n", texture_name(texture));
#endif

    glActiveTexture(texture_unit);
    glBindTexture(rgl_tex->gl_target, 0);
}
