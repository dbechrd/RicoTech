#ifndef RICO_INTERNAL_MATERIAL_H
#define RICO_INTERNAL_MATERIAL_H

// TODO: Material describes texture(s), device state settings, and which v/f
//       shader to use when rendering.
struct rico_material
{
    struct uid uid;

    //TODO: vert_shader, frag_shader

    pkid tex_albedo;
    pkid tex_mrao;
    pkid tex_emission;
    // TODO: pkid tex_normal;
};

static void material_bind(pkid pkid);
static void material_unbind(pkid pkid);

#endif
