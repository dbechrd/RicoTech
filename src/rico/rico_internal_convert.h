#ifndef RICO_INTERNAL_CONVERT_H
#define RICO_INTERNAL_CONVERT_H

static int rico_convert(int argc, char **argv);
static int rico_convert_obj(const char *filename);
static int load_obj_file_new(const char *filename);

#if 0
static inline char *read_word(char *fp, int len, char **buf)
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