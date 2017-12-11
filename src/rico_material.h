#ifndef RICO_MATERIAL_H
#define RICO_MATERIAL_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_material {
    struct hnd hnd;
    u32 ref_count;

    uid tex_diffuse_uid;
    uid tex_specular_uid;
    struct rico_texture *tex_diffuse;
    struct rico_texture *tex_specular;

    float shiny;
};
extern struct rico_material *RICO_DEFAULT_MATERIAL;

void material_fixup(struct rico_material *material)
{
    material->tex_diffuse = hashtable_search_uid(&global_uids,
                                                 material->tex_diffuse_uid);
    material->tex_specular = hashtable_search_uid(&global_uids,
                                                  material->tex_specular_uid);
}

int material_init(struct rico_material *material, const char *name,
                  struct rico_texture *tex_diffuse,
                  struct rico_texture *tex_specular, float shiny);
void material_free(struct rico_material *material);
void material_bind(struct rico_material *material);
void material_unbind(struct rico_material *material);
//SERIAL(material_serialize_0);
//DESERIAL(material_deserialize_0);

#endif // RICO_MATERIAL_H
