#ifndef RICO_HND_H
#define RICO_HND_H

typedef u32 pkid;

#define RICO_HND_TYPES(f)	 \
    f(RICO_HND_NULL,	  0) \
    f(RICO_HND_OBJECT,    sizeof(struct rico_object))   \
    f(RICO_HND_TEXTURE,   sizeof(struct rico_texture))	\
    f(RICO_HND_MESH,      sizeof(struct rico_mesh))		\
    f(RICO_HND_FONT,      sizeof(struct rico_font))		\
    f(RICO_HND_STRING,    sizeof(struct rico_string))	\
    f(RICO_HND_MATERIAL,  sizeof(struct rico_material))	\
    f(RICO_HND_BBOX,      sizeof(struct bbox))			\
    f(RICO_HND_HASHTABLE, sizeof(struct hash_table))	\
	f(RICO_HND_COUNT,	  0)

enum rico_hnd_type
{
    RICO_HND_TYPES(GEN_LIST)
};

struct uid
{
    pkid pkid;
    enum rico_hnd_type type;
    char name[32];
};

#endif
