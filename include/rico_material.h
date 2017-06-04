#ifndef RICO_MATERIAL_H
#define RICO_MATERIAL_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_material {
    struct rico_uid uid;
    u32 ref_count;

    u32 tex_diffuse;
    u32 tex_specular;

    float shiny;
};
extern const u32 RICO_MATERIAL_SIZE;

extern u32 RICO_DEFAULT_MATERIAL;

u32 material_request(enum rico_persist persist, u32 handle);
int material_request_by_name(u32 *_handle, enum rico_persist persist,
                             const char *name);
int material_init(u32 *_handle, enum rico_persist persist, const char *name,
                  u32 tex_diffuse, u32 tex_specular, float shiny);
void material_free(enum rico_persist persist, u32 handle);
inline const char *material_name(enum rico_persist persist, u32 handle);
inline float material_shiny(enum rico_persist persist, u32 handle);
void material_bind(enum rico_persist persist, u32 handle);
void material_unbind(enum rico_persist persist, u32 handle);
//SERIAL(material_serialize_0);
//DESERIAL(material_deserialize_0);

#endif // RICO_MATERIAL_H