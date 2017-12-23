#ifndef RICO_MATERIAL_H
#define RICO_MATERIAL_H

struct rico_material {
    u32 id;
    float shiny;
    u32 tex_diffuse_id;
    u32 tex_specular_id;

    u32 name_offset;
};
extern struct pool_id RICO_DEFAULT_MATERIAL;

global const char *material_name(struct rico_material *material);
void material_bind(struct rico_material *material);
void material_unbind(struct rico_material *material);

#endif // RICO_MATERIAL_H
