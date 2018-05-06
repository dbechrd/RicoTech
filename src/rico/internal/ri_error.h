#ifndef RICO_INTERNAL_ERROR_H
#define RICO_INTERNAL_ERROR_H

#include "rico_error.h"

static enum RICO_error rico_error_print(const char *file, int line,
                                        enum RICO_error err, const char *fmt,
                                        ...);
static enum RICO_error rico_fatal_print(const char *file, int line,
                                        enum RICO_error err, const char *fmt,
                                        ...);

#endif