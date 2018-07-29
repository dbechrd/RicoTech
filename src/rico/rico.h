#ifndef RICO_H
#define RICO_H

#include "dlb_types.h"
#define DLB_MATH_PRINT
#include "dlb_math.h"

#include "AL/al.h"
#include "AL/alc.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include "GL/gl3w.h"
#include "SDL/SDL.h"

#include "rico_error.h"
#include "rico_types.h"
#include "rico_aabb.h"
#include "rico_primitives.h"
#include "rico_mesh.h"
#include "rico_light.h"
#include "rico_audio.h"
#include "rico_object.h"
#include "rico_string.h"
#include "rico_state.h"
#include "rico_pack.h"
#include "rico_camera.h"
#include "rico_heiro.h"
#include "rico_ui.h"

// TODO: Use perf timer, not ticks. Expose RICO_timer somehow.
#define PERF_START(name) u32 perf_##name = SDL_GetTicks()
#define PERF_END(name) \
    printf("[PERF][%s][%u ticks]\n", #name, SDL_GetTicks() - perf_##name);
#define PERF_END_MSG(name, fmt, ...) \
    printf("[PERF][%s][%u ticks] ", #name, SDL_GetTicks() - perf_##name); \
    printf(fmt, ##__VA_ARGS__);

extern int RICO_init();
extern void RICO_window_size(s32 *x, s32 *y);
extern void RICO_cleanup();

#endif