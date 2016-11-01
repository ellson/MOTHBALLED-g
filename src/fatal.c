/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "fatal.h"

/**
 * Report an error and exit.
 * Intended only for use from FATAL(...) macro.
 *
 * Example:
 *
 *     if (!(buf = malloc(sz)))
 *         FATAL("malloc()");
 *
 * might produce the error:
 *     FATAL at foo.c:123 with error 12 "Not enough space" : malloc()
 *
 * @param file name of the source file, as provided by __FILE__
 * @param line within the source file, as provided by __LINE__
 * @param format for message string
 * @param ... varargs for the message string.
 *
 */
void fatal(const char *file, int line, const char *format, ...)
{
    va_list args;
    int err = errno;

    fprintf(stderr, "FATAL at %s:%d", file, line);
    if (err) {
        fprintf(stderr, " with error %d \"%s\"", err, strerror(err));
    }
    fprintf(stderr, " : ");
    va_start(args, format);
    fprintf(stderr, format, args);
    va_end(args);
    putc('\n', stderr);

    exit(EXIT_FAILURE);
}
