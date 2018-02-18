#ifndef RICO_INTERNAL_CONVERT_H
#define RICO_INTERNAL_CONVERT_H

int rico_convert(int argc, char **argv);
int rico_convert_obj(const char *filename);
int load_obj_file_new(const char *filename);

internal inline char *strsep(char **stringp, const char delim)
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

internal inline char *strsep_mul(char **stringp, const char *delims)
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

internal inline long fast_atol(const char *str)
{
    if (!str) return 0;

    long val = 0;
    while(*str) {
        val = val*10 + (*str++ - '0');
    }
    return val;
}

#if 0
internal inline char *read_word(char *fp, int len, char **buf)
{
    len--;
    while (*fp && len)
    {
        (*buf)++;
    }
    **buf = '\0';

    return fp;
}
#endif

#endif