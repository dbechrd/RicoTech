#ifndef RICO_H
#define RICO_H

#if defined(__GNUC__) || defined(__clang__)
#include <x86intrin.h>
#endif

#include <stdint.h>
#include "const.h"

//#include <stdlib.h>
#include "3rdparty/GL/gl3w.h"
#include "3rdparty/SDL/SDL.h"
#include "3rdparty/AL/al.h"
#include "3rdparty/AL/alc.h"
#include "3rdparty/MurmurHash3.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

struct pack;

#include "dlb_string.h"
#include "geom.h"
#include "util.h"
#include "rico_file.h"
#include "rico_cereal.h"
//#include "rico_pool.h"
#include "rico_hnd.h"
#include "rico_hash.h"
#include "rico_audio.h"

#include "bbox.h"
#include "camera.h"
#include "program.h"
#include "rico_light.h"
#include "rico_object.h"
#include "rico_texture.h"
#include "rico_mesh.h"
#include "rico_font.h"
#include "rico_string.h"
#include "rico_material.h"
//#include "rico_chunk.h"
#include "pack_builder.h"

#include "primitives.h"
#include "regularpoly.h"
#include "rico_convert.h"
#include "rico_collision.h"
#include "rico_physics.h"
#include "rico_state.h"
#include "shader.h"
#include "glref.h"

#ifdef __APPLE__
#define __gl_h_
//#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

//#include "3rdparty/CacheLineSize.h"
//#include "3rdparty/nuklear.h"
//#include "3rdparty/nuklear_sdl_gl3.h"

#define STBI_ONLY_TGA
#include "3rdparty/stb_image.h"
#include "3rdparty/tinyobjloader.h"
#include "3rdparty/MurmurHash3.h"

#endif