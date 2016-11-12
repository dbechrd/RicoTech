#include "rico_texture.h"
#include "rico_uid.h"
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

static int build_texture(struct rico_texture *tex, const void *pixels);
static int texture_load_file(uint32 handle, GLenum target, const char *filename);
static int texture_load_pixels(uint32 handle, GLenum target, int width,
                               int height, int bpp, const void *pixels);

//TODO: Allocate from heap pool, not stack
#define POOL_SIZE 50
static struct rico_texture pool[POOL_SIZE];

//TODO: Instantiate pool[0] (handle 0) with a special default object
//      that can be used to visually represent a NULL object in-game
static uint32 next_handle = 1;

int texture_set_default(GLenum target, const char *filename)
{
    return texture_load_file(RICO_TEXTURE_DEFAULT, target, filename);
}

int make_texture_file(GLenum target, const char *filename, uint32 *_handle)
{
    //TODO: Handle out-of-memory
    //TODO: Implement reuse of pool objects
    if (next_handle >= POOL_SIZE)
    {
        fprintf(stderr, "Out of memory: Texture pool exceeded max size of %d.\n",
                POOL_SIZE);

        //TODO: Proper error codes
        *_handle = RICO_TEXTURE_DEFAULT;
        return 1; //ERR_OUT_OF_MEMORY
    }

    int err = texture_load_file(next_handle, target, filename);
    if (err) {
        *_handle = RICO_TEXTURE_DEFAULT;
        return err;
    }

    *_handle = next_handle++;
    return 0;
}

int make_texture_pixels(GLenum target, int width, int height, int bpp,
                        const void *pixels, uint32 *_handle)
{
    //TODO: Handle out-of-memory
    //TODO: Implement reuse of pool objects
    if (next_handle >= POOL_SIZE)
    {
        fprintf(stderr, "Out of memory: Texture pool exceeded max size of %d.\n",
                POOL_SIZE);

        //TODO: Proper error codes
        *_handle = RICO_TEXTURE_DEFAULT;
        return 1; //ERR_OUT_OF_MEMORY
    }

    int err = texture_load_pixels(next_handle, target, width, height, bpp,
                                  pixels);
    if (err) {
        *_handle = RICO_TEXTURE_DEFAULT;
        return err;
    }

    *_handle = next_handle++;
    return 0;
}

static int texture_load_file(uint32 handle, GLenum target, const char *filename)
{
    int err = 0;

    struct rico_texture *tex = &pool[handle];
    tex->gl_target = target;

    //--------------------------------------------------------------------------
    // Load raw texture data
    //--------------------------------------------------------------------------
    unsigned char* pixels =
        stbi_load(filename, &tex->width, &tex->height, &tex->bpp, 4);
    tex->bpp = 32;

    if (!pixels)
    {
        fprintf(stderr, "Failed to load texture file: %s", filename);
        err = 1; //ERR_FILE
        goto cleanup;
    }

    if (build_texture(tex, pixels))
    {
        fprintf(stderr, "Failed to build GL texture: %s", filename);
        err = 1; //ERR_FILE
    }

cleanup:
    stbi_image_free(pixels);
    return err;
}

static int texture_load_pixels(uint32 handle, GLenum target, int width,
                               int height, int bpp, const void *pixels)
{
    struct rico_texture *tex = &pool[handle];
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
        assert(0);
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
    return 0;
}

void texture_bind(uint32 handle)
{
    glBindTexture(pool[handle].gl_target, pool[handle].gl_id);
}

void texture_unbind(uint32 handle)
{
    glBindTexture(pool[handle].gl_target, 0);
}

void free_texture(uint32 *handle)
{
    glDeleteTextures(1, &pool[*handle].gl_id);

    //Hack: How to mark this as deleted?
    pool[*handle].gl_id = 9999999;
    *handle = RICO_TEXTURE_DEFAULT;
}