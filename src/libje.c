/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

// private includes
#include "ikea.h"
#include "emit.h"

// public include
#include "libje.h"

/**
 * initialize context and process file args
 *
 * @param pargc
 * @param argv
 * @param optind
 * @return context
 */
PARSE_t *je_initialize(int *pargc, char *argv[], int optind)
{
    PARSE_t *PARSE;

    if (! (PARSE = calloc(1, sizeof(PARSE_t))))
        fatal_perror("Error - calloc(): ");

    PARSE->progname = argv[0];
    PARSE->out = stdout;

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0) {    // No file args,  default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    PARSE->TOKEN.pargc = pargc;
    PARSE->TOKEN.argv = argv;

    // gather session info, including starttime.
    //    subsequent calls to je_session() just reuse the info gathered in this first call.
    je_session(PARSE);
    
    // create (or reopen) store for the containers
    PARSE->ikea_store = ikea_store_open( NULL );

    emit_initialize(PARSE);

    return PARSE;
}

/**
 * finalize and free context
 *
 * @param PARSE context
 */
void je_finalize( PARSE_t * PARSE )
{
   emit_finalize(PARSE);

   ikea_store_snapshot(PARSE->ikea_store);
   ikea_store_close(PARSE->ikea_store);

   // free context
   free(PARSE);
}

void je_interrupt( PARSE_t * PARSE )
{
   ikea_store_close(PARSE->ikea_store);
}
