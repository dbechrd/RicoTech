#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "const.h"
#include "geom.h"

struct camera;

#define RICO_PRIMITIVES(f) \
    f(PRIM_LINE)           \
    f(PRIM_COUNT)

enum rico_prim {
    RICO_PRIMITIVES(GEN_LIST)
};
extern const char *rico_prim_string[];

struct prim_vertex {
    struct vec3 pos;
    struct col4 col;
};

struct line {
    struct prim_vertex vertices[2];
};

int prim_init(enum rico_prim prim);
void prim_draw_line(const struct line *line, const struct camera *camera,
                    const struct mat4 *model_matrix, struct col4 color);

#endif // PRIMITIVES_H