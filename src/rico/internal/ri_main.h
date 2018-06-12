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

#include "ri_const.h"
#include "ri_error.h"
struct pack; // Cleanup: Can we move these to the top of this file or something?

#include "ri_util.h"
#include "ri_file.h"
//#include "ri_pool.h"
#include "ri_hnd.h"
#include "ri_hash.h"
#include "ri_audio.h"

#include "ri_bbox.h"
#include "ri_camera.h"
#include "ri_program.h"
#include "ri_light.h"
#include "ri_object.h"
#include "ri_texture.h"
#include "ri_mesh.h"
#include "ri_font.h"
#include "ri_string.h"
#include "ri_material.h"
//#include "ri_rico_chunk.h"
#include "ri_pack.h"

#include "ri_primitives.h"
#include "ri_regularpoly.h"
#include "ri_convert.h"
#include "ri_collision.h"
#include "ri_physics.h"
#include "ri_state.h"
#include "ri_shader.h"
#include "ri_glref.h"
#include "ri_ui.h"
#include "ri_cereal.h"

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
