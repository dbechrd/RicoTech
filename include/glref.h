#ifndef GLREF_H
#define GLREF_H

#include "geom.h"
#include "camera.h"
#include "rico_mesh.h"

void init_glref(struct rico_mesh **meshes, int mesh_count);
void select_next_obj();
void select_prev_obj();
void translate_selected(struct vec4 offset);
void rotate_selected(struct vec4 offset);
void duplicate_selected();
void update_glref(GLfloat dt, bool ambient_light);
void render_glref();
void free_glref();

#endif // !GLREF_H