#ifndef BULLET_H
#define BULLET_H

#include <GL/gl3w.h>
#include "rect.h"

typedef struct Bullet {
    Rect *rect;
    Vec3 vel;
    Vec3 acc;
} Bullet;

Bullet *Bullet_create(Rect *rect);
void Bullet_destroy(Bullet *bullet);

void Bullet_update(Bullet *bullet);
void Bullet_render(Bullet *bullet);

#endif