#include "geom.h"

const struct col4 COLOR_TRANSPARENT = { 0.0f, 0.0f, 0.0f, 0.0f };
const struct col4 COLOR_BLACK   = { 0.0f, 0.0f, 0.0f, 1.0f };
const struct col4 COLOR_RED     = { 1.0f, 0.0f, 0.0f, 1.0f };
const struct col4 COLOR_GREEN   = { 0.0f, 1.0f, 0.0f, 1.0f };
const struct col4 COLOR_BLUE    = { 0.0f, 0.0f, 1.0f, 1.0f };
const struct col4 COLOR_YELLOW  = { 1.0f, 1.0f, 0.0f, 1.0f };
const struct col4 COLOR_CYAN    = { 0.0f, 1.0f, 1.0f, 1.0f };
const struct col4 COLOR_MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };
const struct col4 COLOR_WHITE   = { 1.0f, 1.0f, 1.0f, 1.0f };
const struct col4 COLOR_GRAY    = { 0.5f, 0.5f, 0.5f, 1.0f };

const struct col4 COLOR_BLACK_HIGHLIGHT   = { 0.0f, 0.0f, 0.0f, 0.7f };
const struct col4 COLOR_RED_HIGHLIGHT     = { 1.0f, 0.0f, 0.0f, 0.7f };
const struct col4 COLOR_GREEN_HIGHLIGHT   = { 0.0f, 1.0f, 0.0f, 0.7f };
const struct col4 COLOR_BLUE_HIGHLIGHT    = { 0.0f, 0.0f, 1.0f, 0.7f };
const struct col4 COLOR_YELLOW_HIGHLIGHT  = { 1.0f, 1.0f, 0.0f, 0.7f };
const struct col4 COLOR_CYAN_HIGHLIGHT    = { 0.0f, 1.0f, 1.0f, 0.7f };
const struct col4 COLOR_MAGENTA_HIGHLIGHT = { 1.0f, 0.0f, 1.0f, 0.7f };
const struct col4 COLOR_WHITE_HIGHLIGHT   = { 1.0f, 1.0f, 1.0f, 0.7f };
const struct col4 COLOR_GRAY_HIGHLIGHT    = { 0.5f, 0.5f, 0.5f, 0.7f };

const struct col4 COLOR_DARK_RED     = { 0.5f, 0.0f, 0.0f, 1.0f };
const struct col4 COLOR_DARK_GREEN   = { 0.0f, 0.5f, 0.0f, 1.0f };
const struct col4 COLOR_DARK_BLUE    = { 0.0f, 0.0f, 0.5f, 1.0f };
const struct col4 COLOR_DARK_YELLOW  = { 0.5f, 0.5f, 0.0f, 1.0f };
const struct col4 COLOR_DARK_CYAN    = { 0.0f, 0.5f, 0.5f, 1.0f };
const struct col4 COLOR_DARK_MAGENTA = { 0.5f, 0.0f, 0.5f, 1.0f };
const struct col4 COLOR_DARK_GRAY    = { 0.25f, 0.25f, 0.25f, 1.0f };

const struct vec3 VEC3_ZERO  = { 0.0f, 0.0f, 0.0f };
const struct vec3 VEC3_UNIT  = { 1.0f, 1.0f, 1.0f };
const struct vec3 VEC3_X     = { 1.0f, 0.0f, 0.0f };
const struct vec3 VEC3_Y     = { 0.0f, 1.0f, 0.0f };
const struct vec3 VEC3_Z     = { 0.0f, 0.0f, 1.0f };
const struct vec3 VEC3_RIGHT = { 1.0f, 0.0f, 0.0f };
const struct vec3 VEC3_UP    = { 0.0f, 1.0f, 0.0f };
const struct vec3 VEC3_FWD   = { 0.0f, 0.0f,-1.0f };
const struct vec3 VEC3_SMALL = { 0.01f, 0.01f, 0.01f };

const struct mat4 MAT4_IDENT = {{{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
}}};

const struct quat QUAT_IDENT = { 1.0f, {{ 0.0f, 0.0f, 0.0f }} };
