#ifndef RICO_PACK_H
#define RICO_PACK_H

extern pkid global_default_font;
extern pkid global_default_font_texture;
extern pkid global_default_texture_diff;
extern pkid global_default_texture_spec;
extern pkid global_default_texture_emis;
extern pkid global_default_material;
extern pkid global_default_mesh_cube;
extern pkid global_default_mesh_sphere;

#define PKID_BLOB_BITS 24
#define PKID_BLOB_MASK ((1 << PKID_BLOB_BITS) - 1)
#define PKID_PACK_MASK (0xffffffff ^ PKID_BLOB_MASK)
#define PKID_PACK(id) ((id & PKID_PACK_MASK) >> PKID_BLOB_BITS)
#define PKID_BLOB(id) (id & PKID_BLOB_MASK)
#define PKID_GENERATE(pack, blob) ((pack << PKID_BLOB_BITS) | blob)

extern u32 ric_pack_init(u32 pack_id, const char *name, u32 blob_count,
                         u32 buffer_size);
extern void *ric_pack_lookup(pkid pkid);
extern void *ric_pack_lookup_by_name(u32 pack_id, const char *name);
extern pkid ric_pack_first(u32 pack_id);
extern pkid ric_pack_last(u32 pack_id);
extern pkid ric_pack_next(pkid id);
extern pkid ric_pack_prev(pkid id);
extern pkid ric_pack_next_loop(pkid id);
extern pkid ric_pack_prev_loop(pkid id);
extern pkid ric_pack_first_type(u32 pack_id, enum ric_asset_type type);
extern pkid ric_pack_last_type(u32 pack_id, enum ric_asset_type type);
extern pkid ric_pack_next_type(pkid id, enum ric_asset_type type);
extern pkid ric_pack_prev_type(pkid id, enum ric_asset_type type);
extern pkid ric_pack_next_type_loop(pkid id, enum ric_asset_type type);
extern pkid ric_pack_prev_type_loop(pkid id, enum ric_asset_type type);
extern int ric_pack_save(u32 pack_id, bool shrink);
extern int ric_pack_save_as(u32 pack_id, const char *filename, bool shrink);
extern int ric_pack_load(const char *filename, u32 *_pack);
extern void ric_pack_free(u32 pack_id);
extern pkid ric_load_object(u32 pack_id, u32 type, u32 min_size,
                            const char *name);
extern pkid ric_load_texture(u32 pack_id, const char *name, GLenum target,
                             u32 width, u32 height, u8 bpp, u8 *pixels);
extern pkid ric_load_texture_file(u32 pack_id, const char *name,
                                  const char *filename);
extern pkid ric_load_texture_color(u32 pack_id, const char *name,
                                   const struct vec4 *color);
extern pkid ric_load_material(u32 pack_id, const char *name, pkid tex_albedo,
                              pkid tex_mrao, pkid tex_emission);
extern pkid ric_load_font_file(u32 pack_id, const char *name,
                               const char *filename);
extern pkid ric_load_mesh(u32 pack_id, const char *name, u32 vertex_size,
                          u32 vertex_count, const void *vertex_data,
                          u32 element_count, const GLuint *element_data,
                          enum ric_shader_type prog_type);
extern pkid ric_load_string(u32 pack_id, enum ric_string_slot slot, float x,
                            float y, struct vec4 color, u32 lifespan,
                            pkid font, const char *text);
extern int ric_load_obj_file(u32 pack_id, const char *filename,
                             pkid *_last_mesh_id, enum ric_shader_type prog_type);

#endif