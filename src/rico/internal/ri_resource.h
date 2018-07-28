#ifndef RICO_INTERNAL_RESOURCE_H
#define RICO_INTERNAL_RESOURCE_H

#include "dlb_hash.h"

extern struct dlb_hash global_fonts;
extern struct dlb_hash global_materials;
extern struct dlb_hash global_meshes;
extern struct dlb_hash global_textures;

static void rico_resource_init();

#endif
