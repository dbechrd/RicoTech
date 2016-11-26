#include "rico_texture.h"
#include "rico_uid.h"
#include "rico_pool.h"
#include "stb_image.h"
#include <stdlib.h>
#include <stdio.h>

struct rico_texture {
    struct rico_uid uid;
    GLuint gl_id;
    GLenum gl_target;

    //TODO: What's the point of saving this if we don't also save pixel data?
    GLsizei width;
    GLsizei height;
    GLsizei bpp;
};

uint32 RICO_TEXTURE_DEFAULT = 0;
static struct rico_pool textures;

static int build_texture(struct rico_texture *tex, const void *pixels);

int rico_texture_init(uint32 pool_size)
{
    return pool_init("Textures", pool_size, sizeof(struct rico_texture),
                     &textures);
}

int texture_load_file(const char *name, GLenum target, const char *filename,
                      uint32 *_handle)
{
    int err;
    *_handle = RICO_TEXTURE_DEFAULT;

    err = pool_alloc(&textures, _handle);
    if (err) return err;

    struct rico_texture *tex = pool_read(&textures, *_handle);
    uid_init(name, &tex->uid);
    tex->gl_target = target;

    // Load raw texture data
    unsigned char* pixels = stbi_load(filename, &tex->width, &tex->height,
                                      &tex->bpp, 4);
    if (!pixels)
    {
        fprintf(stderr, "Failed to load texture file: %s", filename);
        err = ERR_TEXTURE_LOAD;
        goto cleanup;
    }

    // Requested bit depth is 32-bit, but stbi_load returns original depth of
    // image file in bytes.
    tex->bpp = 32;

    // Build GL texture
    err = build_texture(tex, pixels);

cleanup:
    stbi_image_free(pixels);
    return err;
}

int texture_load_pixels(const char *name, GLenum target, int width, int height,
                        int bpp, const void *pixels, uint32 *_handle)
{
    int err;
    *_handle = RICO_TEXTURE_DEFAULT;

    err = pool_alloc(&textures, _handle);
    if (err) return err;

    struct rico_texture *tex = pool_read(&textures, *_handle);
    uid_init(name, &tex->uid);
    tex->gl_target = target;
    tex->width = width;
    tex->height = height;
    tex->bpp = bpp;

    return build_texture(tex, pixels);
}

static int build_texture(struct rico_texture *tex, const void *pixels)
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
    glCreateTextures(tex->gl_target, 1, &tex->gl_id);
    glBindTexture(tex->gl_target, tex->gl_id);

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
    glTexParameteri(tex->gl_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(tex->gl_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

    // For GL_CLAMP_TO_BORDER
    //float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

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
    glTexParameteri(tex->gl_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(tex->gl_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
    GLenum format;
    switch (tex->bpp)
    {
    case 8: // Greyscale
        format = GL_RED;
        break;
    case 24: // RGB
        format = GL_RGB;
        break;
    case 32: // RGBA
        format = GL_RGBA;
        break;
    default: // Unsupported BPP
        return ERR_TEXTURE_UNSUPPORTED_BPP;
    }

    glTexImage2D(tex->gl_target, 0, format, tex->width, tex->height, 0, format,
                 GL_UNSIGNED_BYTE, pixels);

    //--------------------------------------------------------------------------
    // Generate mipmaps
    //--------------------------------------------------------------------------
    glGenerateMipmap(tex->gl_target);

    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------

    // Unbind the texture
    glBindTexture(tex->gl_target, 0);
    return SUCCESS;
}

void texture_free(uint32 handle)
{
    struct rico_texture *tex = pool_read(&textures, handle);
    glDeleteTextures(1, &tex->gl_id);
    tex->uid = UID_NULL;

    pool_free(&textures, handle);
}

const char *texture_name(uint32 handle)
{
    struct rico_texture *tex = pool_read(&textures, handle);
    return tex->uid.name;
}

void texture_bind(uint32 handle)
{
    // TODO: When is this useful in the context of this application?
    //glActiveTexture(GL_TEXTURE0);

    struct rico_texture *tex = pool_read(&textures, handle);
    glBindTexture(tex->gl_target, tex->gl_id);
}

void texture_unbind(uint32 handle)
{
    struct rico_texture *tex = pool_read(&textures, handle);
    glBindTexture(tex->gl_target, 0);
}

struct rico_pool *texture_pool_unsafe()
{
    return &textures;
}