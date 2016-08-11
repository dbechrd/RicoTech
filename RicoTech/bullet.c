#include <stdlib.h>
#include "bullet.h"

struct bullet *make_bullet(struct rect *rect)
{
    struct bullet *bullet = calloc(1, sizeof(struct bullet));

    bullet->rect = rect;

    bullet->vel = (struct vec4) { 0.0f, 0.0f, 0.0f, 1.0f };
    bullet->acc = (struct vec4) { 0.0f, 0.0f, 0.0f, 1.0f };

    return bullet;
}

void free_bullet(struct bullet *bullet)
{
    free(bullet->rect);
    bullet->rect = NULL;
    free(bullet);
    bullet = NULL;
}

void set_bullet_size(struct bullet *bullet, GLfloat width, GLfloat height, GLfloat d)
{
    set_rect_size(bullet->rect, width, height, false);
}

void set_bullet_pos(struct bullet *bullet, GLfloat x, GLfloat y, GLfloat z)
{
    set_rect_pos(bullet->rect, x, y, z, false);
}

void update_bullet(struct bullet *bullet)
{
    bullet->vel.x += bullet->acc.x;
    bullet->vel.y += bullet->acc.y;
    bullet->vel.z += bullet->acc.z;

    bullet->vel.x += bullet->acc.x;
    bullet->vel.y += bullet->acc.y;
    bullet->vel.z += bullet->acc.z;

    set_rect_pos(bullet->rect,
                 bullet->rect->pos.x + bullet->vel.x,
                 bullet->rect->pos.y + bullet->vel.y,
                 bullet->rect->pos.z + bullet->vel.z, true);
}

void render_bullet(struct bullet *bullet)
{
    render_rect(bullet->rect);
}