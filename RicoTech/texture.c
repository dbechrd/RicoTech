#include "texture.h"
#include "stb_image.h"
#include <stdlib.h>
#include <stdio.h>

struct texture *make_texture(GLenum target, const char *filename)
{
    struct texture *tex;

    tex = (struct texture *)malloc(sizeof(struct texture));

    tex->target = target;

    //--------------------------------------------------------------------------
    // Load raw texture data
    //--------------------------------------------------------------------------
    unsigned char* pixels = stbi_load(filename, &tex->width, &tex->height,
                                      &tex->bpp, 4);
    if (!pixels)
    {
        fprintf(stderr, "Failed to load texture: %s", filename);
        return NULL;
    }

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
    glCreateTextures(tex->target, 1, &tex->texture);
    glBindTexture(tex->target, tex->texture);

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
    glTexParameteri(tex->target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(tex->target, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
    glTexParameteri(tex->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(tex->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //--------------------------------------------------------------------------
    // Load raw data into texture
    //--------------------------------------------------------------------------
    glTexImage2D(tex->target, 0, GL_RGBA, tex->width, tex->height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);

    //--------------------------------------------------------------------------
    // Generate mipmaps
    //--------------------------------------------------------------------------
    glGenerateMipmap(tex->target);

    //--------------------------------------------------------------------------
    // Clean up
    //--------------------------------------------------------------------------
    
    // Free the raw data
    stbi_image_free(pixels);

    // Unbind the texture
    glBindTexture(tex->target, 0);

    return tex;
}

void free_texture(struct texture **tex)
{
    glDeleteTextures(1, &(*tex)->texture);
    free(*tex);
    *tex = NULL;
}