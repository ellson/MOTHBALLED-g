/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "fatal.h"

void fatal(const char *file, int line, const char *format, ...)
{
    va_list argList;
    int err = errno;

    fprintf(stderr, "FATAL at %s:%d", file, line);
    if (err) {
        fprintf(stderr, " with error %d: \"%s\"", err, strerror(err));
    }
    if (format) {
        fprintf(stderr, " : ");
        va_start(argList, format);
        fprintf(stderr, format, argList);
        va_end(argList);
    }
    putc('\n', stderr);

    exit(EXIT_FAILURE);
}
