#ifndef RICO_HEIRO_H
#define RICO_HEIRO_H

struct RICO_heiro_string
{
    u32 length;
    u32 vertex_count;
    struct text_vertex *vertices;
    struct rect bounds;
};

extern void RICO_heiro_build(struct RICO_heiro_string *string,
                             struct rect *cursor, const u8 *str, u32 len,
                             u32 cur);
extern void RICO_heiro_render(struct RICO_heiro_string *string, s32 sx, s32 sy,
                              const struct vec4 *color);
extern void RICO_heiro_free(struct RICO_heiro_string *string);

#endif