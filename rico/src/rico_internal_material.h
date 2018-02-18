#ifndef RICO_INTERNAL_MATERIAL_H
#define RICO_INTERNAL_MATERIAL_H

struct rico_material
{
    struct uid uid;
    pkid tex_id[3];
};

void material_bind(pkid pkid);
void material_unbind(pkid pkid);

#endif
