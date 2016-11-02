#ifndef CONST_H
#define CONST_H

//TODO: Is this the correct header? Is this dependency necessary?
#include <SDL/SDL_config.h>

//------------------------------------------------------------------------------
// Basic type definitions
//------------------------------------------------------------------------------
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// TODO: Platform-specific definitions?
typedef unsigned char uchar;
typedef unsigned long ulong;

//------------------------------------------------------------------------------
// Math / Physics constants
//------------------------------------------------------------------------------
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

#define M_SEVENTH_DEG 51.428571428571428571428571428571

//------------------------------------------------------------------------------
// Error codes
//------------------------------------------------------------------------------
#define ERR_GL_LINK_FAILED GL_FALSE
#define ERR_GL_ATTRIB_NOTFOUND -1

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define sizeof_member(type, member) sizeof(((type *)0)->member)

#ifdef RICO_DEBUG
#define assert(exp) if(!(exp)) {*(int*)0=0;}
#else
#define assert(exp)
#endif

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------
enum { VBO_VERTEX, VBO_ELEMENT };

#endif // CONST_H