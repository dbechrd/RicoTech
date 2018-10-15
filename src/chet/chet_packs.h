#ifndef CHET_PACKS_H
#define CHET_PACKS_H

#include "chet.h"

void pack_build_default(struct pack_info **pack_table, const char *filename);
void pack_build_alpha(struct pack_info **pack_table, const char *filename);
void pack_build_clash_of_cubes(struct pack_info **pack_table, const char *filename);

#endif