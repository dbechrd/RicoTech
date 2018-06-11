#ifndef RICO_H
#define RICO_H

#include "dlb_types.h"
#define DLB_MATH_PRINT
#include "dlb_math.h"
#include "SDL/SDL.h"
#include "GL/gl3w.h"

#include "rico_error.h"
#include "rico_hnd.h"
#include "rico_bbox.h"
#include "rico_primitives.h"
#include "rico_mesh.h"
#include "rico_light.h"
#include "rico_audio.h"
#include "rico_object.h"
#include "rico_string.h"
#include "rico_state.h"
#include "rico_pack.h"
#include "rico_camera.h"
#include "rico_ui.h"

extern int RICO_init();
extern void RICO_window_size(s32 *x, s32 *y);
extern void RICO_cleanup();

#endif