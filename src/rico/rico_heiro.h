#ifndef RICO_HEIRO_H
#define RICO_HEIRO_H

struct RICO_heiro_string
{
    struct rect bounds;
    u32 length;
    u32 vertex_count;
    struct text_vertex *verts;
};

extern void RICO_heiro_build(struct RICO_heiro_string **result,
                             struct rect *cursor, const struct dlb_string *str,
                             u32 cur);
extern void RICO_heiro_render(struct RICO_heiro_string *string, s32 sx, s32 sy,
                              const struct vec4 *color);
extern void RICO_heiro_free(struct RICO_heiro_string *string);

#endif