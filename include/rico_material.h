#ifndef RICO_MATERIAL_H
#define RICO_MATERIAL_H

// IMPORTANT: *DO NOT* add pointers in this struct, it will break cereal!
struct rico_material {
    struct rico_uid uid;
    u32 ref_count;

    struct hnd tex_diffuse;
    struct hnd tex_specular;

    float shiny;
};
extern const u32 RICO_MATERIAL_SIZE;

extern struct hnd RICO_DEFAULT_MATERIAL;

struct hnd material_request(struct hnd handle);
int material_request_by_name(struct hnd *_handle, enum rico_persist persist,
                             const char *name);
int material_init(struct hnd *_handle, enum rico_persist persist,
                  const char *name, struct hnd tex_diffuse,
                  struct hnd tex_specular, float shiny);
void material_free(struct hnd handle);
inline const char *material_name(struct hnd handle);
inline float material_shiny(struct hnd handle);
void material_bind(struct hnd handle);
void material_unbind(struct hnd handle);
//SERIAL(material_serialize_0);
//DESERIAL(material_deserialize_0);

#endif // RICO_MATERIAL_H