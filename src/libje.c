/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

#include "libje_private.h"

/**
 * initiaze context and process file args
 *
 * @param pargc
 * @param argv
 * @param optind
 * @return context
 */
context_t *je_initialize(int *pargc, char *argv[], int optind)
{
    context_t *C;

    if (! (C = calloc(1, sizeof(context_t))))
        fatal_perror("Error - calloc(): ");

    C->progname = argv[0];
    C->out = stdout;

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0) {    // No file args,  default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    C->pargc = pargc;
    C->argv = argv;

    // gather session info, including starttime.
    //    subsequent calls to je_session() just reuse the info gathered in this first call.
    je_session(C);
    
    emit_initialize(C);
    return C;
}

/**
 * finalize and free context
 *
 * @param C context
 */
void je_finalize(context_t *C)
{
   emit_finalize(C);

   // free context
   free(C);
}
