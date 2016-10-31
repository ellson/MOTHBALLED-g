/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "fatal.h"

void fatal_printf(const char *format, ...)
{
    va_list argList;

    va_start(argList, format);
    fprintf(stderr, format, argList);
    va_end(argList);

    exit(EXIT_FAILURE);
}

void fatal_perror(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

void fatal(const char *file, int line, const char *format, ...)
{
    va_list argList;
    int err = errno;

    fprintf(stderr, "FATAL at %s:%d", file, line);
    if (err) {
        fprintf(stderr, " with error: %s", strerror(err));
    }
    if (format) {
        va_start(argList, format);
        fprintf(stderr, format, argList);
        va_end(argList);
    }
    putc('\n', stderr);

    exit(EXIT_FAILURE);
}
