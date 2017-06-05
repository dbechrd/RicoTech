#ifndef RICO_H
#define RICO_H

#ifdef __APPLE__
#define __gl_h_
//#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

#include <stdint.h>
//#include <stdlib.h>
#include "const.h"
#include "3rdparty/GL/gl3w.h"

#include <math.h>
#include <stdio.h>

#include "geom.h"
#include "util.h"
#include "rico_uid.h"
#include "rico_file.h"
#include "rico_cereal.h"
#include "rico_pool.h"
#include "rico_hash.h"
#include "bbox.h"
#include "camera.h"
#include "glref.h"
#include "load_object.h"
#include "MurmurHash3.h"
#include "primitives.h"
#include "program.h"
#include "regularpoly.h"
#include "rico_chunk.h"
#include "rico_collision.h"
#include "rico_font.h"
#include "rico_light.h"
#include "rico_material.h"
#include "rico_mesh.h"
#include "rico_object.h"
#include "rico_physics.h"
#include "rico_state.h"
#include "rico_string.h"
#include "rico_texture.h"
#include "shader.h"

#include "3rdparty/CacheLineSize.h"
//#include "3rdparty/nuklear.h"
//#include "3rdparty/nuklear_sdl_gl3.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_TGA
#include "3rdparty/stb_image.h"
#include "3rdparty/tinyobjloader.h"
#include "3rdparty/MurmurHash3.h"
#include "3rdparty/SDL/SDL.h"

#endif // RICO_H
