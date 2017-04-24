#include "const.h"
#include "SDL/SDL.h"
#include <stdio.h>
#include <string.h>

const char *rico_error_string[] = {
    RICO_ERRORS(GEN_STRING)
};

enum rico_error rico_error_print(enum rico_error err, const char *desc,
                                 const char *file, int line)
{
    if (err != SUCCESS)
    {
        printf("[ERROR][%s:%d][%d:%s] %s\n", strstr(file, "src"), line, err,
               rico_error_string[err], desc);
    }
    return err;
}

enum rico_error rico_fatal_print(enum rico_error err, const char *desc,
                                 const char *file, int line)
{
    if (err != SUCCESS)
    {
        printf("[FATAL][%s:%d][%d:%s] %s\n", strstr(file, "src"), line, err,
               rico_error_string[err], desc);

        char buf[500] = { 0 };
        sprintf(buf, "%s : %d\n%s (%d)", strstr(file, "src"), line,
                rico_error_string[err], err);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", buf,
                                 NULL);
    }
    return err;
}