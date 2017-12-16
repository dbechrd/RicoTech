#ifndef RICO_MATERIAL_H
#define RICO_MATERIAL_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_material {
    struct hnd hnd;
    u32 ref_count;

    struct pool_id tex_diffuse_id;
    struct pool_id tex_specular_id;

    float shiny;
};
extern struct pool_id RICO_DEFAULT_MATERIAL;

int material_init(struct rico_material *material, const char *name,
                  struct pool_id tex_diffuse_id, struct pool_id tex_specular_id,
                  float shiny);
int material_free(struct rico_material *material);
void material_bind(struct rico_material *material);
void material_unbind(struct rico_material *material);
//SERIAL(material_serialize_0);
//DESERIAL(material_deserialize_0);

#endif // RICO_MATERIAL_H
