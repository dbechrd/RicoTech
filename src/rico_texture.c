struct rico_texture *RICO_DEFAULT_TEXTURE_DIFF;
struct rico_texture *RICO_DEFAULT_TEXTURE_SPEC;

internal int build_texture(struct rico_texture *texture, const void *pixels);

int texture_load_file(struct rico_texture *texture, const char *name,
                      GLenum target, const char *filename, u32 bpp)
{
#if RICO_DEBUG_TEXTURE
    printf("[ tex][load] filename=%s\n", filename);
#endif

    enum rico_error err;
    int width, height, depth;

    // Load raw texture data
    unsigned char* pixels = stbi_load(filename, &width, &height, &depth, 4);
    if (!pixels)
    {
        err = RICO_ERROR(ERR_TEXTURE_LOAD, "Failed to load texture file: %s",
                         filename);
        goto cleanup;
    }

    // Load pixels
    err = texture_load_pixels(texture, name, target, width, height, bpp,
                              pixels);

cleanup:
    stbi_image_free(pixels);
    return err;
}

int texture_load_pixels(struct rico_texture *texture, const char *name,
                        GLenum target, u32 width, u32 height, u32 bpp,
                        const void *pixels)
{
    enum rico_error err;

#if RICO_DEBUG_TEXTURE
    printf("[ tex][init] name=%s\n", name);
#endif

    // Note: If we want to serialize texture data we have to store the filename
    //       or the pixel data in the struct.
    hnd_init(&texture->hnd, RICO_HND_TEXTURE, name);
    texture->gl_target = target;
    texture->width = width;
    texture->height = height;
    texture->bpp = bpp;

    err = build_texture(texture, pixels);
    if (err) return err;

    // Store in global hash table
    err = hashtable_insert_hnd(&global_textures, &texture->hnd, texture);
    return err;
}

internal int build_texture(struct rico_texture *texture, const void *pixels)
{
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
                 texture->height, 0, format, GL_UNSIGNED_BYTE, pixels);

    //--------------------------------------------------------------------------
    // Generate mipmaps
    //--------------------------------------------------------------------------
    glGenerateMipmap(texture->gl_target);

    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    // Unbind the texture
    glBindTexture(texture->gl_target, 0);
    return SUCCESS;
}

void texture_free(struct rico_texture *texture)
{
    if (texture->ref_count > 0)
        texture->ref_count--;

#if RICO_DEBUG_TEXTURE
    printf("[ tex][ rls] uid=%d ref=%d name=%s\n", texture->hnd.uid,
           texture->ref_count, texture->hnd.name);
#endif

    if (texture->ref_count > 0)
        return;

    // TODO: Use fixed pool slots or request and never release at initialize
    //if (handle == RICO_DEFAULT_TEXTURE_DIFF)
    //    return;
    //if (handle == RICO_DEFAULT_TEXTURE_SPEC)
    //    return;

#if RICO_DEBUG_TEXTURE
    printf("[ tex][free] uid=%d name=%s\n", texture->hnd.uid,
           texture->hnd.name);
#endif

    hashtable_delete_hnd(&global_textures, &texture->hnd);

    glDeleteTextures(1, &texture->gl_id);
    pool_remove(texture->hnd.pool, texture->hnd.id);
}

void texture_bind(struct rico_texture *texture, GLenum texture_unit)
{
    glActiveTexture(texture_unit);
    glBindTexture(texture->gl_target, texture->gl_id);
}

void texture_unbind(struct rico_texture *texture, GLenum texture_unit)
{
    if (!texture) return;

    glActiveTexture(texture_unit);
    glBindTexture(texture->gl_target, 0);
}
