#ifndef RICO_MATERIAL_H
#define RICO_MATERIAL_H

#include "rico_pool.h"

extern u32 RICO_MATERIAL_DEFAULT;

int rico_material_init(u32 pool_size);
int material_request(u32 handle);
int material_init(const char *name, u32 tex_diffuse, u32 tex_specular,
                  float shiny, u32 *_handle);
void material_free(u32 handle);
const char *material_name(u32 handle);
float material_shiny_get(u32 handle);
void material_bind(u32 handle);
void material_unbind(u32 handle);
int material_serialize_0(const void *handle, const struct rico_file *file);
int material_deserialize_0(void *_handle, const struct rico_file *file);

struct rico_pool *material_pool_get_unsafe();
void material_pool_set_unsafe(struct rico_pool *pool);

#endif // RICO_MATERIAL_H