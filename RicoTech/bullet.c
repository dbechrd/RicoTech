#include "bullet.h"

#include <stdlib.h>

Bullet *Bullet_create(Rect *rect)
{
    Bullet *bullet = malloc(sizeof(Bullet));

    bullet->rect = rect;

    //TODO: Dunno what the point of Vector4 is, probably not necessary
    bullet->vel = (Vec3) { 0.0f, 0.0f, 0.0f };
    bullet->acc = (Vec3) { 0.0f, 0.0f, 0.0f };

    return bullet;
}

void Bullet_destroy(Bullet *bullet)
{
    free(bullet->rect);
    bullet->rect = NULL;
    free(bullet);
    bullet = NULL;
}

void Bullet_update(Bullet *bullet)
{
    bullet->vel.x += bullet->acc.x;
    bullet->vel.y += bullet->acc.y;
    bullet->vel.z += bullet->acc.z;

    bullet->vel.x += bullet->acc.x;
    bullet->vel.y += bullet->acc.y;
    bullet->vel.z += bullet->acc.z;

    Rect_move(bullet->rect,
        bullet->rect->x + bullet->vel.x,
        bullet->rect->y + bullet->vel.y,
        bullet->rect->z + bullet->vel.z);

}

void Bullet_render(Bullet *bullet)
{
    Rect_render(bullet->rect);
}