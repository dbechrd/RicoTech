#include "rico_material.h"
#include "rico_texture.h"
#include "rico_uid.h"
#include "rico_pool.h"
// #include "stb_image.h"
// #include <stdlib.h>
// #include <stdio.h>

struct rico_material {
    struct rico_uid uid;
    u32 ref_count;

    u32 tex_diffuse;
    u32 tex_specular;
    float shiny;
};

u32 RICO_MATERIAL_DEFAULT = 0;
static struct rico_pool materials;

int rico_material_init(u32 pool_size)
{
    return pool_init("Materials", pool_size, sizeof(struct rico_material), 0,
                     &materials);
}

// TODO: Do proper reference counting, this function is stupid.
int material_request(u32 handle)
{
    struct rico_material *material = pool_read(&materials, handle);
    material->ref_count++;
    return handle;
}

int material_init(const char *name, u32 tex_diffuse, u32 tex_specular,
                  float shiny, u32 *_handle)
{
#ifdef RICO_DEBUG_MATERIAL
    printf("[Material] Init %s\n", name);
#endif

    enum rico_error err;
    *_handle = RICO_MATERIAL_DEFAULT;

    err = pool_alloc(&materials, _handle);
    if (err) return err;

    struct rico_material *material = pool_read(&materials, *_handle);

    uid_init(&material->uid, RICO_UID_MATERIAL, name, false);
    material->tex_diffuse = texture_request(tex_diffuse);
    material->tex_specular = texture_request(tex_specular);
    material->shiny = shiny;

    return err;
}

void material_free(u32 handle)
{
    struct rico_material *material = pool_read(&materials, handle);

#ifdef RICO_DEBUG_MATERIAL
    printf("[Material] Free %s\n", material->uid.name);
#endif

    material->ref_count--;
    if (material->ref_count > 0)
        return;

    texture_free(material->tex_diffuse);
    texture_free(material->tex_specular);

    material->uid.uid = UID_NULL;
    pool_free(&materials, handle);
}

const char *material_name(u32 handle)
{
    struct rico_material *material = pool_read(&materials, handle);
    return material->uid.name;
}

float material_shiny_get(u32 handle)
{
    struct rico_material *material = pool_read(&materials, handle);
    return material->shiny;
}

void material_bind(u32 handle)
{
    struct rico_material *material = pool_read(&materials, handle);
    texture_bind(material->tex_diffuse, GL_TEXTURE0);
    texture_bind(material->tex_specular, GL_TEXTURE1);
}

void material_unbind(u32 handle)
{
    struct rico_material *material = pool_read(&materials, handle);
    texture_unbind(material->tex_diffuse, GL_TEXTURE0);
    texture_unbind(material->tex_specular, GL_TEXTURE1);
}

struct rico_pool *material_pool_unsafe()
{
    return &materials;
}