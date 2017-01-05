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

struct rico_pool *material_pool_unsafe();

#endif // RICO_MATERIAL_H