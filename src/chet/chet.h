#ifndef CHET_H
#define CHET_H

#define DLB_MATH_IMPLEMENTATION
#include "rico.h"

enum game_object_type
{
    OBJ_TIMMY = RICO_OBJECT_TYPE_COUNT,
    OBJ_GAME_PANEL,
    OBJ_GAME_BUTTON,
    OBJ_RAY_VISUALIZER,
    OBJ_SMALL_CUBE,
    OBJ_COUNT
};

struct timmy
{
    struct RICO_object rico;
    bool lights_on;
    bool audio_on;
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
    struct vec3 acc;
    struct vec3 vel;
    bool resting;
    bool collide_aabb;
    bool collide_obb;
};

struct pack_info
{
    const char *path_pak;
    const char *path_sav;
    u32 pak_id;
    u32 sav_id;
};

#endif