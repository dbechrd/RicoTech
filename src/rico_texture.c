global const char *texture_name(struct rico_texture *tex)
{
    return (u8 *)tex + tex->name_offset;
}
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

    //--------------------------------------------------------------------------
    // Generate textures
    //--------------------------------------------------------------------------
    /*************************************************************************
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
    glCreateTextures(texture->gl_target, 1, &texture->gl_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(texture->gl_target, texture->gl_id);

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
    glTexParameteri(texture->gl_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(texture->gl_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
    glTexParameteri(texture->gl_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(texture->gl_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
    GLenum format_internal;
    GLenum format;
    switch (texture->bpp)
    {
    case 8: // Greyscale (assume not gamma-corrected???)
        format          = GL_RED;
        format_internal = GL_RED;
        break;
    case 24: // RGB
        format          = GL_RGB;
        // TODO: Store processed texture data in linear space and change this
        //       back to GL_RGB.
        format_internal = GL_SRGB;
        break;
    case 32: // RGBA
        format          = GL_RGBA;
        // TODO: Store processed texture data in linear space and change this
        //       back to GL_RGBA.
        format_internal = GL_SRGB_ALPHA;
        break;
    default: // Unsupported BPP
        return RICO_ERROR(ERR_TEXTURE_UNSUPPORTED_BPP, NULL);
    }

    glTexImage2D(texture->gl_target, 0, format_internal, texture->width,
                 texture->height, 0, format, GL_UNSIGNED_BYTE,
                 texture_pixels(texture));

    //--------------------------------------------------------------------------
    // Generate mipmaps
    //--------------------------------------------------------------------------
    glGenerateMipmap(texture->gl_target);

    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    // Unbind the texture
    glBindTexture(texture->gl_target, 0);

    return err;
}

void texture_delete(struct rico_texture *texture)
{
#if RICO_DEBUG_TEXTURE
    printf("[ tex][ del] name=%s\n", texture_name(texture));
#endif

    glDeleteTextures(1, &texture->gl_id);
}

void texture_bind(struct rico_texture *texture, GLenum texture_unit)
{
#if RICO_DEBUG_TEXTURE
    printf("[ tex][bind] name=%s\n", texture_name(texture));
#endif

    RICO_ASSERT(texture);
    if (texture->gl_id)
    {
        texture_upload(texture);
    }

    glActiveTexture(texture_unit);
    glBindTexture(texture->gl_target, texture->gl_id);
}

void texture_unbind(struct rico_texture *texture, GLenum texture_unit)
{
#if RICO_DEBUG_TEXTURE
    printf("[ tex][unbd] name=%s\n", texture_name(texture));
#endif

    RICO_ASSERT(texture);
    RICO_ASSERT(texture->gl_id);

    glActiveTexture(texture_unit);
    glBindTexture(texture->gl_target, 0);
}
