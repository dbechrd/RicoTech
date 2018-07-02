#ifndef RICO_HEIRO_H
#define RICO_HEIRO_H

extern void RICO_heiro_build(struct rect *bounds, struct rect *cursor,
                             const u8 *str, u32 len, u32 cur);
extern void RICO_heiro_render(s32 sx, s32 sy);

#endif