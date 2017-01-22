#include "const.h"
#include <stdio.h>

const char *rico_error_string[] = {
    RICO_ERRORS(GEN_STRING)
};

enum rico_error rico_error_print(enum rico_error err, const char *file,
                                 int line)
{
    if (err != SUCCESS)
    {
        printf("[ERROR][%s:%d][%d:%s]\n", file, line, err,
                rico_error_string[err]);
#ifdef RICO_DEBUG_ERROR_ASSERT
        RICO_ASSERT(0);
#endif
    }
    return err;
}