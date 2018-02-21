#ifndef RICO_INTERNAL_ERROR_H
#define RICO_INTERNAL_ERROR_H

#include "RICO/rico_error.h"

enum rico_error rico_error_print(const char *file, int line,
    enum rico_error err, const char *fmt, ...);
enum rico_error rico_fatal_print(const char *file, int line,
    enum rico_error err, const char *fmt, ...);

#endif