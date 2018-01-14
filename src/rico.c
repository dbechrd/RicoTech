#include "rico.h"
#include "gl3w.c"

#include "const.c"
#include "geom.c"
#include "util.c"
#include "rico_hash.c"
#include "rico_audio.c"

#include "rico_hnd.c"
#include "shader.c"
#include "program.c"
#include "camera.c"
#include "bbox.c"

#include "rico_cereal.c"
#include "rico_material.c"
#include "rico_object.c"
#include "glref.c"

#include "primitives.c"
#include "regularpoly.c"
#include "rico_chunk.c"
#include "rico_collision.c"
#include "rico_convert.c"
#include "rico_font.c"
//#include "rico_light.c"
#include "rico_mesh.c"
#include "rico_physics.c"
#include "rico_pool.c"
#include "pack_builder.c"

#include <time.h>
#include "rico_state.c"

#include "rico_string.c"
#include "rico_texture.c"
#include "tests.c"
#include "rico_file.c"

//#include "3rdparty/CacheLineSize.c"
//#include "3rdparty/main_nuke.c"
#include "3rdparty/MurmurHash3.c"

#define STB_IMAGE_IMPLEMENTATION
#include "3rdparty/stb_image.h"
