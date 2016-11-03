/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "fatal.h"

/**
 * Report an error and exit.
 * Intended only for use from the FATAL(...) macro, #define'd in fatal.h
 *
 * Source example 1 - basic:
 *     if (!(buf = malloc(sz)))
 *         FATAL("malloc()");
 *
 * produces error messages like:
 *     FATAL at foo.c:123 with error 12 "Not enough space" : malloc()
 *
 * Source example 2 - using additional va_args:
 *     if (!(fh = fopen(fn, "r")))
 *         FATAL("fopen(\"%s\", \"r\")", fn);
 * 
 * produces error messages like:
 *     FATAL at foo.c:456 with error 2 "No such file or directory" : fopen("../a/b/c", "r")
 *
 * FIXME - include program_invocation_short_name in error messages
 *         see "man 3 program_invocation_short_name"
 *         but this is a GNU extension so will need help from configure.ac to make portable
 *         Alternatively, main's argv[0], which we keep in PARSE->progname,
 *
 * @param file name of the source file, as provided by __FILE__
 * @param line within the source file, as provided by __LINE__
 * @param format for message string
 * @param ... varargs for the format string.
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
    vfprintf(stderr, format, args);
    va_end(args);
    putc('\n', stderr);

    exit(EXIT_FAILURE);
}
