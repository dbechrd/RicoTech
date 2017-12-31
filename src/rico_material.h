#ifndef RICO_MATERIAL_H
#define RICO_MATERIAL_H

struct rico_material
{
    u32 id;
    u32 tex_id[2];

    u32 name_offset;
};

global const char *material_name(struct rico_material *material);
void material_bind(struct pack *pack, u32 id);
void material_unbind(struct pack *pack, u32 id);

#endif // RICO_MATERIAL_H
