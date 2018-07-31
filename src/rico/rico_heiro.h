#ifndef RICO_HEIRO_H
#define RICO_HEIRO_H

extern void ric_heiro_build(struct ric_heiro_string **result,
                            struct rect *cursor, const struct dlb_string *str,
                            u32 cur);
extern void ric_heiro_render(struct ric_heiro_string *string, s32 sx, s32 sy,
                             const struct vec4 *color);
extern void ric_heiro_free(struct ric_heiro_string *string);

#endif