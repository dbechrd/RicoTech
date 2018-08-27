#ifndef DLB_STRING_H
#define DLB_STRING_H

#include "dlb_types.h"

#define STR(str) (const struct dlb_string) { str, sizeof(str) }
#define STRL(str, len) (const struct dlb_string) { str, len }

#if 0
typedef struct Intern {
    size_t len;
    const char *str;
} Intern;

Arena str_arena;
Intern *interns;

const char *str_intern_range(const char *start, const char *end) {
    size_t len = end - start;
    for (Intern *it = interns; it != buf_end(interns); it++) {
        if (it->len == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }
    char *str = arena_alloc(&str_arena, len + 1);
    memcpy(str, start, len);
    str[len] = 0;
    buf_push(interns, (Intern){ len, str });
    return str;
}

const char *str_intern(const char *str) {
    return str_intern_range(str, str + strlen(str));
}
#endif

struct dlb_string
{
    const char *s;
    u32 len;
};

static inline u32 dlb_strlen(const char *str)
{
	u32 len = 0;
    while (*str++)
    {
        len++;
    }
	return len;
}

static inline char *dlb_strsep_c(char **stringp, const char delim)
{
    char *start = *stringp;

    while (**stringp)
    {
        if (**stringp == delim)
        {
            **stringp = '\0';
            (*stringp)++;
            break;
        }
        (*stringp)++;
    }

    return start;
}

static inline char *dlb_strsep(char **stringp, const char *delims)
{
    char *start = *stringp;
    const char *delim;

    while (**stringp)
    {
        delim = delims;
        while (delim)
        {
            if (**stringp == *delim)
            {
                **stringp = '\0';
                (*stringp)++;
                return start;
            }
            delim++;
        }
        (*stringp)++;
    }

    return start;
}

static inline long dlb_atol(const char *str)
{
    if (!str) return 0;

    long val = 0;
    while(*str)
    {
        val = val*10 + (*str++ - '0');
    }

    return val;
}

#if 0
struct dlb_string
{
    u32 buffer_len;
    const char *str;
};

struct dlb_string *dlb_string_alloc(const char *str)
{
    u32 buffer_len = strlen(str);
    struct dlb_string *s = calloc(1, sizeof(struct dlb_string) + strlen(str) +
                                  1);
    s->buffer_len = buffer_len;
    s->str = s + 1;
    return s;
}
#endif

#endif