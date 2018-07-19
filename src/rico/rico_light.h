#ifndef RICO_LIGHT_H
#define RICO_LIGHT_H

extern bool RICO_lighting_enabled;

enum RICO_light_type
{
    LIGHT_AMBIENT       = 0,
    LIGHT_DIRECTIONAL   = 1,
    LIGHT_POINT         = 2,
    LIGHT_SPOT          = 3
};

struct RICO_light
{
    enum RICO_light_type type;
    bool on;
    struct vec3 col;
    struct vec3 pos;
    float intensity;

    union
    {
        struct
        {
            struct vec3 dir;
        } directional;

        struct
        {
            // Distance fall-off
            float kc;  // Constant
            float kl;  // Linear
            float kq;  // Quadratic
        } point;

        struct
        {
            struct vec3 dir;

            // Point / Spot lights
            // Distance fall-off
            float kc;  // Constant
            float kl;  // Linear
            float kq;  // Quadratic

            // Angle of inner (full intensity) and outer (fall-off to zero) cone
            float theta_inner;
            float theta_outer;
        } spot;
    };
};

/*
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
*/

#endif