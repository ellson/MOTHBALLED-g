#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "error_functions.h"

void
fatal(const char *format, ...)
{
    va_list argList;

    va_start(argList, format);
    fprintf(stderr, format, argList);
    va_end(argList);

    exit(EXIT_FAILURE);
}

