#ifndef CHET_H
#define CHET_H

#define DLB_MATH_IMPLEMENTATION
#include "RICO/rico.h"

enum game_object_type
{
    OBJ_TIMMY = RICO_OBJECT_TYPE_COUNT,
    OBJ_GAME_PANEL,
    OBJ_GAME_BUTTON,
    OBJ_RAY_VISUALIZER,
    OBJ_SMALL_CUBE,
    OBJ_COUNT
};

enum actions
{
    ACTION_RICO_TEST = ACTION_COUNT,
    ACTION_TYPE_NEXT,
    ACTION_TYPE_PREV
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

struct ray_visualizer
{
    struct RICO_object rico;
    u32 lifetime;
};

static struct game_panel panel_1;

enum audio_type
{
    AUDIO_WELCOME,
    AUDIO_THUNDER,
    AUDIO_BUTTON,
    AUDIO_VICTORY,
    AUDIO_COUNT
};

struct RICO_audio_buffer audio_buffers[AUDIO_COUNT];
struct RICO_audio_source audio_sources[AUDIO_COUNT];

static const char *PATH_ALPHA = "packs/alpha.pak";
static const char *PATH_ALPHA_SAV = "packs/alpha_sav.pak";

//-------------------------------------------------------------------

struct small_cube
{
    struct RICO_object rico;
    struct vec3 acc;
    struct vec3 vel;
    bool resting;
};

static pkid small_cubes[2];

static const float GRAVITY = -0.0098f; // TODO: Scale by delta_time properly
static const float COEF_COLLIDE = 0.15f; // TODO: Elastic collision coef

static const char *PATH_CLASH = "packs/clash.pak";
static const char *PATH_CLASH_SAV = "packs/clash_sav.pak";

#endif