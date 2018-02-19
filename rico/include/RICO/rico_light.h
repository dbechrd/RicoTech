#ifndef RICO_LIGHT_H
#define RICO_LIGHT_H

struct light_ambient
{
    struct vec3 color;
};

struct light_directional
{
    struct vec3 color;
    struct vec3 direction;
};

struct light_point
{
    struct vec3 color;
    struct vec3 position;
    float intensity;

    // Distance fall-off
    float kc;  // Constant
    float kl;  // Linear
    float kq;  // Quadratic
};

struct light_spot
{
    struct vec3 color;
    struct vec3 position;
    struct vec3 direction;

    // Angle of inner (full intensity) and outer (fall-off to zero) cone
    float theta_inner;
    float theta_outer;

    // Distance fall-off
    float kc;  // Constant
    float kl;  // Linear
    float kq;  // Quadratic
};

bool RICO_lighting_enabled;

#endif