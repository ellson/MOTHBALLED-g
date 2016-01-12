/* vim:set shiftwidth=4 ts=8 expandtab: */

#include "libje_private.h"

context_t *je_initialize(int *pargc, char *argv[], int optind)
{
    context_t *C;

    if (! (C = calloc(1, sizeof(context_t)))) {
        perror("Error - calloc(): ");
        exit(EXIT_FAILURE);
    }

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

void je_finalize(context_t *C)
{
   emit_finalize(C);

   // free context
   free(C);
}
