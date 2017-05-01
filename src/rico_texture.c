#include "rico_texture.h"
#include "rico_uid.h"
#include "rico_pool.h"
#include "stb_image.h"
#include <stdlib.h>
#include <stdio.h>

struct rico_texture {
    struct rico_uid uid;
    u32 ref_count;

    GLuint gl_id;
    GLenum gl_target;

    //TODO: What's the point of saving this if we don't also save pixel data?
    GLsizei width;
    GLsizei height;
    GLsizei bpp;
};

u32 RICO_TEXTURE_DEFAULT_DIFF = 0;
u32 RICO_TEXTURE_DEFAULT_SPEC = 0;
static struct rico_pool *textures;

static int build_texture(struct rico_texture *tex, const void *pixels);

int rico_texture_init(u32 pool_size)
{
    return pool_init("Textures", pool_size, sizeof(struct rico_texture), 0,
                     &textures);
}

// TODO: Do proper reference counting, this function is stupid. Need to save
//       filename for that to work (how to track load_pixels calls?)
int texture_request(u32 handle)
{
    struct rico_texture *tex = pool_read(textures, handle);
    tex->ref_count++;

#ifdef RICO_DEBUG_TEXTURE
    printf("[ tex][++ %d] name=%s\n", tex->ref_count, tex->uid.name);
#endif

    return handle;
}

int texture_load_file(const char *name, GLenum target, const char *filename,
                      u32 bpp, u32 *_handle)
{
#ifdef RICO_DEBUG_TEXTURE
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
    err = texture_load_pixels(name, target, width, height, bpp, pixels,
                              _handle);

cleanup:
    stbi_image_free(pixels);
    return err;
}

int texture_load_pixels(const char *name, GLenum target, u32 width, u32 height,
                        u32 bpp, const void *pixels, u32 *_handle)
{
#ifdef RICO_DEBUG_TEXTURE
    printf("[ tex][init] name=%s\n", name);
#endif

    enum rico_error err;

    err = pool_handle_alloc(&textures, _handle);
    if (err) return err;

    struct rico_texture *tex = pool_read(textures, *_handle);

    // Note: If we want to serialize texture data we have to store the filename
    //       or the pixel data in the struct.
    uid_init(&tex->uid, RICO_UID_TEXTURE, name, false);
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
    glActiveTexture(GL_TEXTURE0);
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
    //glTexParameteri(tex->gl_target, GL_TEXTURE_WRAP_R, GL_REPEAT);

    // For GL_CLAMP_TO_BORDER
    //float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    //glTexParameterfv(tex->gl_target, GL_TEXTURE_BORDER_COLOR, color);

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
        return RICO_ERROR(ERR_TEXTURE_UNSUPPORTED_BPP, NULL);
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

void texture_free(u32 handle)
{
    // TODO: Use static pool slots
    if (handle == RICO_TEXTURE_DEFAULT_DIFF)
        return;
    if (handle == RICO_TEXTURE_DEFAULT_SPEC)
        return;

    struct rico_texture *tex = pool_read(textures, handle);
    if (tex->ref_count > 0)
        tex->ref_count--;

#ifdef RICO_DEBUG_TEXTURE
    printf("[ tex][-- %d] name=%s\n", tex->ref_count, tex->uid.name);
#endif

    if (tex->ref_count > 0)
        return;

#ifdef RICO_DEBUG_TEXTURE
    printf("[ tex][free] name=%s\n", tex->uid.name);
#endif

    glDeleteTextures(1, &tex->gl_id);

    tex->uid.uid = UID_NULL;
    pool_handle_free(textures, handle);
}

const char *texture_name(u32 handle)
{
    struct rico_texture *tex = pool_read(textures, handle);
    return tex->uid.name;
}

void texture_bind(u32 handle, GLenum texture_unit)
{
    // TODO: When is this useful in the context of this application?
    struct rico_texture *tex = pool_read(textures, handle);
    glActiveTexture(texture_unit);
    glBindTexture(tex->gl_target, tex->gl_id);
}

void texture_unbind(u32 handle, GLenum texture_unit)
{
    struct rico_texture *tex = pool_read(textures, handle);
    glActiveTexture(texture_unit);
    glBindTexture(tex->gl_target, 0);
}

struct rico_pool *texture_pool_unsafe()
{
    return textures;
}