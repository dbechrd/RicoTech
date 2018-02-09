#ifndef DLB_STRING_H
#define DLB_STRING_H

#include "dlb_types.h"

static inline u32 dlb_strlen(const char *str)
{
	u32 len = 0;
	while (*str) len++;
	return len;
}

#if 0
struct dlb_string
{
    u32 len;
    const char *str;
};

struct dlb_string *dlb_string_alloc(const char *str)
{
    u32 len = strlen(str);
    struct dlb_string *s = calloc(1, sizeof(struct dlb_string) + strlen(str) + 1);
    s->len = len;
    s->str = s + 1;
    return s;
}
#endif

#endif