#include "geom.h"

const struct col4 COLOR_BLACK   = { 0.0f, 0.0f, 0.0f, 1.0f };
const struct col4 COLOR_RED     = { 1.0f, 0.0f, 0.0f, 1.0f };
const struct col4 COLOR_GREEN   = { 0.0f, 1.0f, 0.0f, 1.0f };
const struct col4 COLOR_BLUE    = { 0.0f, 0.0f, 1.0f, 1.0f };
const struct col4 COLOR_YELLOW  = { 1.0f, 1.0f, 0.0f, 1.0f };
const struct col4 COLOR_CYAN    = { 0.0f, 1.0f, 1.0f, 1.0f };
const struct col4 COLOR_MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };
const struct col4 COLOR_WHITE   = { 1.0f, 1.0f, 1.0f, 1.0f };
const struct col4 COLOR_GRAY    = { 0.5f, 0.5f, 0.5f, 1.0f };

const struct vec4 VEC4_ZERO = { 0 };
const struct vec4 VEC4_UNIT = { 1.0f, 1.0f, 1.0f, 0.0f }; //TODO: W is 1 or 0?