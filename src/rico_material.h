#ifndef RICO_MATERIAL_H
#define RICO_MATERIAL_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_material {
    struct hnd hnd;
    u32 ref_count;

    struct rico_texture *tex_diffuse;
    struct rico_texture *tex_specular;

    float shiny;
};
extern const u32 RICO_MATERIAL_SIZE;

global struct rico_material *RICO_DEFAULT_MATERIAL;

int material_init(struct rico_material *material, const char *name,
                  struct rico_texture *tex_diffuse,
                  struct rico_texture *tex_specular, float shiny);
void material_free(struct rico_material *material);
internal inline const char *material_name(struct rico_material *material);
internal inline float material_shiny(struct rico_material *material);
void material_bind(struct rico_material *material);
void material_unbind(struct rico_material *material);
//SERIAL(material_serialize_0);
//DESERIAL(material_deserialize_0);

#endif // RICO_MATERIAL_H
