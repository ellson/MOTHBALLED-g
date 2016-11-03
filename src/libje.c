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
SESSION_t *initialize(int *pargc, char *argv[], int optind)
{
    SESSION_t *SESSION;
    PARSE_t *PARSE;

    if (! (SESSION = calloc(1, sizeof(SESSION_t))))
        FATAL("calloc()");

    SESSION->progname = argv[0];
    SESSION->out = stdout;

    if (! (PARSE = calloc(1, sizeof(PARSE_t))))
        FATAL("calloc()");

    PARSE->SESSION = SESSION;
    SESSION->PARSE = PARSE;
#if 1
    PARSE->progname = argv[0];
    PARSE->out = stdout;
#endif

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0) {    // No file args,  default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    PARSE->TOKEN.pargc = pargc;
    PARSE->TOKEN.argv = argv;

    // gather session info, including starttime.
    //    subsequent calls to session() just reuse the info gathered in this first call.
    session(SESSION);
    
    // create (or reopen) store for the containers
    SESSION->ikea_store = ikea_store_open( NULL );
#if 1
    PARSE->ikea_store = ikea_store_open( NULL );
#endif

//    emit_initialize(PARSE);

    return SESSION;
}

/**
 * finalize and free context
 *
 * @param PARSE context
 */
void finalize( SESSION_t * SESSION )
{
   PARSE_t *PARSE = SESSION->PARSE;
   emit_finalize(PARSE);

   ikea_store_snapshot(PARSE->ikea_store);
   ikea_store_close(PARSE->ikea_store);

   // free context
   free(PARSE);
   free(SESSION);
}

void interrupt( SESSION_t * SESSION )
{
   ikea_store_close(SESSION->PARSE->ikea_store);
}
