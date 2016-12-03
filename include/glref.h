#ifndef GLREF_H
#define GLREF_H

#include "geom.h"
#include "camera.h"
#include "rico_mesh.h"

int init_glref();
int init_hardcoded_test_chunk(uint32 *meshes, uint32 mesh_count);
void select_next_obj();
void select_prev_obj();
void translate_selected(struct vec4 offset);
void rotate_selected(struct vec4 offset);
int duplicate_selected();
void delete_selected();
void update_glref(GLfloat dt, bool ambient_light);
void render_glref();
void free_glref();

#endif // !GLREF_H