#include "rico_material.h"
#include "rico_texture.h"
#include "rico_uid.h"
#include "rico_pool.h"
#include <malloc.h>
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
static struct rico_pool *materials;

int rico_material_init(u32 pool_size)
{
    materials = calloc(1, sizeof(*materials));
    return pool_init("Materials", pool_size, sizeof(struct rico_material), 0,
                     materials);
}

// TODO: Do proper reference counting, this function is stupid.
int material_request(u32 handle)
{
    struct rico_material *material = pool_read(materials, handle);
    material->ref_count++;

#ifdef RICO_DEBUG_MATERIAL
    printf("[ mtl][++ %d] name=%s\n", material->ref_count, material->uid.name);
#endif

    return handle;
}

int material_init(const char *name, u32 tex_diffuse, u32 tex_specular,
                  float shiny, u32 *_handle)
{
#ifdef RICO_DEBUG_MATERIAL
    printf("[ mtl][init] name=%s\n", name);
#endif

    enum rico_error err;
    *_handle = RICO_MATERIAL_DEFAULT;

    err = pool_handle_alloc(materials, _handle);
    if (err) return err;

    struct rico_material *material = pool_read(materials, *_handle);

    uid_init(&material->uid, RICO_UID_MATERIAL, name, true);
    material->tex_diffuse = texture_request(tex_diffuse);
    material->tex_specular = texture_request(tex_specular);
    material->shiny = shiny;

    return err;
}

void material_free(u32 handle)
{
    // TODO: Use static pool slots
    if (handle == RICO_MATERIAL_DEFAULT)
        return;

    struct rico_material *material = pool_read(materials, handle);
    if (material->ref_count > 0)
        material->ref_count--;

#ifdef RICO_DEBUG_MATERIAL
    printf("[ mtl][-- %d] name=%s\n", material->ref_count, material->uid.name);
#endif

    if (material->ref_count > 0)
        return;

#ifdef RICO_DEBUG_MATERIAL
    printf("[ mtl][free] name=%s\n", material->uid.name);
#endif

    texture_free(material->tex_diffuse);
    texture_free(material->tex_specular);

    material->uid.uid = UID_NULL;
    pool_handle_free(materials, handle);
}

const char *material_name(u32 handle)
{
    struct rico_material *material = pool_read(materials, handle);
    return material->uid.name;
}

float material_shiny_get(u32 handle)
{
    struct rico_material *material = pool_read(materials, handle);
    return material->shiny;
}

void material_bind(u32 handle)
{
    struct rico_material *material = pool_read(materials, handle);
    texture_bind(material->tex_diffuse, GL_TEXTURE0);
    texture_bind(material->tex_specular, GL_TEXTURE1);
}

void material_unbind(u32 handle)
{
    struct rico_material *material = pool_read(materials, handle);
    texture_unbind(material->tex_diffuse, GL_TEXTURE0);
    texture_unbind(material->tex_specular, GL_TEXTURE1);
}

//int material_serialize_0(const void *handle, const struct rico_file *file)
SERIAL(material_serialize_0)
{
    const struct rico_material *mat = handle;

    fwrite(&mat->ref_count,    sizeof(mat->ref_count),    1, file->fs);
    fwrite(&mat->tex_diffuse,  sizeof(mat->tex_diffuse),  1, file->fs);
    fwrite(&mat->tex_specular, sizeof(mat->tex_specular), 1, file->fs);
    fwrite(&mat->shiny,        sizeof(mat->shiny),        1, file->fs);
    return SUCCESS;
}

//int material_deserialize_0(void *_handle, const struct rico_file *file)
DESERIAL(material_deserialize_0)
{
    u32 diffuse, specular;

    struct rico_material *mat = _handle;
    fread(&mat->ref_count, sizeof(mat->ref_count),    1, file->fs);
    fread(&diffuse,        sizeof(mat->tex_diffuse),  1, file->fs);
    fread(&specular,       sizeof(mat->tex_specular), 1, file->fs);
    fread(&mat->shiny,     sizeof(mat->shiny),        1, file->fs);

    mat->tex_diffuse  = texture_request(diffuse);
    mat->tex_specular = texture_request(specular);
    return SUCCESS;
}

struct rico_pool *material_pool_get_unsafe()
{
    return materials;
}

void material_pool_set_unsafe(struct rico_pool *pool)
{
    materials = pool;
}