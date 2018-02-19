#ifndef RICO_PACK_H
#define RICO_PACK_H

struct blob_index
{
    enum rico_hnd_type type;
    u32 offset;
    u32 size;
};

// Memory layout
// -----------------------
// struct pack;
// struct pack_entry index[index_count];
// struct pack_entry fast_index[index_count];
// u8 data[data_size];
struct pack
{
    char magic[4];
    u32 version;
    u32 id;
    char name[32];

    u32 blob_current_id;
    u32 blob_count;
    u32 blobs_used;

    u32 buffer_size;
    u32 buffer_used;

    u32 lookup_offset;
    u32 index_offset;
    u32 data_offset;

    u32 *lookup;
    struct blob_index *index;
    u8 *buffer;
};

// TODO: Probably shouldn't expose these.. could get messy.
#define MAX_PACKS 32
u32 RICO_packs_next;
struct pack *RICO_packs[MAX_PACKS];

struct pack *RICO_pack_init(u32 id, const char *name, u32 blob_count,
                            u32 buffer_size);
void *RICO_pack_lookup(pkid pkid);
int RICO_pack_save(struct pack *pack, const char *filename, bool shrink);
int RICO_pack_load(const char *filename, struct pack **_pack);
void RICO_pack_free(u32 id);

pkid RICO_load_object(struct pack *pack, u32 type, u32 size, const char *name);
pkid RICO_load_texture(struct pack *pack, const char *name, GLenum target,
                       u32 width, u32 height, u8 bpp, u8 *pixels);
pkid RICO_load_texture_file(struct pack *pack, const char *name,
                            const char *filename);
pkid RICO_load_texture_color(struct pack *pack, const char *name,
                             struct vec4 color);
pkid RICO_load_material(struct pack *pack, const char *name, pkid tex_albedo,
                        pkid tex_mrao, pkid tex_emission);
pkid RICO_load_font(struct pack *pack, const char *name, const char *filename,
                    pkid *font_tex);
pkid RICO_load_mesh(struct pack *pack, const char *name, u32 vertex_size,
                    u32 vertex_count, const void *vertex_data,
                    u32 element_count, const GLuint *element_data);
pkid RICO_load_string(struct pack *pack, enum rico_string_slot slot, float x,
                      float y, struct vec4 color, u32 lifespan,
                      struct rico_font *font, const char *text);
int RICO_load_obj_file(struct pack *pack, const char *filename,
                       pkid *_last_mesh_id);

#endif