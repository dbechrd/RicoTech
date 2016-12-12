#ifndef GLREF_H
#define GLREF_H

#include "geom.h"
#include "camera.h"
#include "rico_mesh.h"

int init_glref();
int init_hardcoded_test_chunk(u32 *meshes, u32 mesh_count);
void select_next_obj();
void select_prev_obj();
void selected_translate(struct camera *camera, const struct vec3 *offset);
void selected_rotate(const struct vec3 *offset);
void selected_scale(const struct vec3 *offset);
int selected_duplicate();
void selected_delete();
void glref_update(GLfloat dt, bool ambient_light);
int glref_debuginfo(const char *str, struct col4 color);
void glref_render(struct camera *camera);
void free_glref();

#endif // !GLREF_H