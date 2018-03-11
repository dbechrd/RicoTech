#ifndef RICO_INTERNAL_H
#define RICO_INTERNAL_H

#if defined(__GNUC__) || defined(__clang__)
#include <x86intrin.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "dlb_types.h"
#include "dlb_string.h"
#include "dlb_math.h"

#include "GL/gl3w.h"
#include "SDL/SDL.h"
//#undef main
#include "AL/al.h"
#include "AL/alc.h"
#include "MurmurHash3.h"

#include "rico_internal_const.h"
#include "rico_internal_error.h"
struct pack; // Cleanup: Can we move these to the top of this file or something?

#include "rico_internal_util.h"
#include "rico_internal_file.h"
//#include "rico_internal_pool.h"
#include "rico_internal_hnd.h"
#include "rico_internal_hash.h"
#include "rico_internal_audio.h"

#include "rico_internal_bbox.h"
#include "rico_internal_camera.h"
#include "rico_internal_program.h"
#include "rico_internal_light.h"
#include "rico_internal_object.h"
#include "rico_internal_texture.h"
#include "rico_internal_mesh.h"
#include "rico_internal_font.h"
#include "rico_internal_string.h"
#include "rico_internal_material.h"
//#include "rico_internal_rico_chunk.h"
#include "rico_internal_pack.h"

#include "rico_internal_primitives.h"
#include "rico_internal_regularpoly.h"
#include "rico_internal_convert.h"
#include "rico_internal_collision.h"
#include "rico_internal_physics.h"
#include "rico_internal_state.h"
#include "rico_internal_shader.h"
#include "rico_internal_glref.h"

#include "RICO/rico_main.h"

#ifdef __APPLE__
#define __gl_h_
//#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

//#include "CacheLineSize.h"
//#include "nuklear.h"
//#include "nuklear_sdl_gl3.h"

#define STBI_ONLY_TGA
#include "stb_image.h"
#include "tinyobjloader.h"
#include "MurmurHash3.h"

static void window_render();

#endif
