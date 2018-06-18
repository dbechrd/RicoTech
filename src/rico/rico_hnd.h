#ifndef RICO_HND_H
#define RICO_HND_H

typedef u32 pkid;

#define RICO_HND_TYPES(f)	 \
    f(RICO_HND_NULL,	  0) \
    f(RICO_HND_OBJECT,    sizeof(struct RICO_object))   \
    f(RICO_HND_TEXTURE,   sizeof(struct RICO_texture))	\
    f(RICO_HND_MESH,      sizeof(struct RICO_mesh))		\
    f(RICO_HND_FONT,      sizeof(struct RICO_font))		\
    f(RICO_HND_STRING,    sizeof(struct RICO_string))	\
    f(RICO_HND_MATERIAL,  sizeof(struct RICO_material))	\
    f(RICO_HND_BBOX,      sizeof(struct RICO_bbox))

enum RICO_hnd_type
{
    RICO_HND_TYPES(GEN_LIST)
    RICO_HND_COUNT
};
extern const char *RICO_hnd_type_string[];
extern const u32 RICO_hnd_type_size[];

typedef u8 buf32[32];

struct uid
{
    pkid pkid;
    enum RICO_hnd_type type;
    buf32 name;
};

#endif