#ifndef BULLET_H
#define BULLET_H

#include <GL/gl3w.h>
#include "rect.h"

struct bullet {
    struct rect *rect;
    struct vec4 vel;
    struct vec4 acc;
};

struct bullet *make_bullet(struct rect *rect);
void free_bullet(struct bullet *);

void set_bullet_size(struct bullet *, GLfloat width, GLfloat height, GLfloat d);
void set_bullet_pos(struct bullet *, GLfloat x, GLfloat y, GLfloat z);
void update_bullet(struct bullet *);
void render_bullet(struct bullet *);

#endif