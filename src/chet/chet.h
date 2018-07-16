#ifndef CHET_H
#define CHET_H

#include "rico.h"

enum game_object_type
{
    OBJ_TIMMY = RICO_OBJECT_TYPE_COUNT,
    OBJ_GAME_PANEL,
    OBJ_GAME_BUTTON,
    OBJ_RAY_VISUALIZER,
    OBJ_SMALL_CUBE,
    OBJ_LIGHT_TEST,
    OBJ_LIGHT_TEST_CUBE,
    OBJ_COUNT
};

struct timmy
{
    struct RICO_object rico;
    bool lights_on;
    bool audio_on;
};

struct light_test
{
    struct RICO_object rico;
};

struct game_button
{
    struct RICO_object rico;
    pkid panel_id;
    u32 index;
};

struct game_panel
{
    struct RICO_object rico;
    pkid buttons[9];
};

struct small_cube
{
    struct RICO_object rico;
};

struct pack_info
{
    const char *path_pak;
    const char *path_sav;
    u32 pak_id;
    u32 sav_id;
};

enum body_type
{
    BODY_PLANE,
    BODY_SPHERE,
    BODY_AABB,
    BODY_OBB,
    BODY_COUNT
};

struct contact
{
    struct vec3 position;
    struct vec3 normal;
    float penetration;
};

struct manifold
{
    struct contact contacts[1];  // TODO: Multiple contact points
    u32 contact_count;
    const struct RICO_object *body_a;
    const struct RICO_object *body_b;
};

#endif