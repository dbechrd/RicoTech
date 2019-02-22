#ifndef RICO_H
#define RICO_H

//#include <stdint.h>
//#include <stdio.h>
//#include <limits.h>
//#include <math.h>
#include <stdlib.h>
//#include <string.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "misc/gl3w.h"

#include "dlb_types.h"
#define DLB_MATH_PRINT
#include "dlb_math.h"
#include "dlb_vector.h"
#include "dlb_string.h"
#include "dlb_hash.h"
#include "dlb_heap.h"

#include "rico_types.h"
#include "rico_error.h"
#include "rico_arena.h"
#include "rico_stream.h"
#include "rico_aabb.h"
#include "rico_primitives.h"
#include "rico_mesh.h"
#include "rico_audio.h"
#include "rico_object.h"
#include "rico_string.h"
#include "rico_state.h"
#include "rico_pack.h"
#include "rico_camera.h"
#include "rico_heiro.h"
#include "rico_ui.h"

extern int ric_init();
extern void ric_window_size(s32 *x, s32 *y);
extern void ric_cleanup();

#endif