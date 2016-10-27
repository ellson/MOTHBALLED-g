/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

// private includes
#include "token.h"
#include "context.h"
#include "ikea.h"
#include "emit.h"

// public include
#include "libje.h"

/**
 * initiaze context and process file args
 *
 * @param pargc
 * @param argv
 * @param optind
 * @return context
 */
CONTEXT_t *je_initialize(int *pargc, char *argv[], int optind)
{
    CONTEXT_t *C;

    if (! (C = calloc(1, sizeof(CONTEXT_t))))
        fatal_perror("Error - calloc(): ");

    C->progname = argv[0];
    C->out = stdout;

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0) {    // No file args,  default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    C->TOKEN.pargc = pargc;
    C->TOKEN.argv = argv;

    // gather session info, including starttime.
    //    subsequent calls to je_session() just reuse the info gathered in this first call.
    je_session(C);
    
    // create (or reopen) store for the containers
    C->ikea_store = ikea_store_open( NULL );

    emit_initialize(C);

    return C;
}

/**
 * finalize and free context
 *
 * @param C context
 */
void je_finalize( CONTEXT_t * C )
{
   emit_finalize(C);

   ikea_store_snapshot(C->ikea_store);
   ikea_store_close(C->ikea_store);

   // free context
   free(C);
}

void je_interrupt( CONTEXT_t * C )
{
   ikea_store_close(C->ikea_store);
}
