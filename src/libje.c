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
#include "container.h"
#include "session.h"

// public include
#include "libje.h"

success_t parse(SESSION_t * SESSION)
{
    GRAPH_t *GRAPH = (GRAPH_t*)(SESSION->CONTAINER);
    success_t rc;

    rc = container(GRAPH);   // FIXME - who allocated GRAPH ??
    return rc;
}

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
    CONTAINER_t *CONTAINER;
    GRAPH_t *GRAPH;

    // FIXME - I think SESSION and CONTAINER can just be onthe stack of THREAD ??
    //
    if (! (SESSION = calloc(1, sizeof(SESSION_t))))
        FATAL("calloc()");

    SESSION->progname = argv[0];
    SESSION->out = stdout;

    if (! (CONTAINER = calloc(1, sizeof(CONTAINER_t))))
        FATAL("calloc()");

    GRAPH = (GRAPH_t*)CONTAINER;
    GRAPH->SESSION = SESSION;
    SESSION->CONTAINER = CONTAINER;
#if 1
    SESSION->progname = argv[0];
    GRAPH->out = stdout;
#endif

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0) {    // No file args,  default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    GRAPH->TOKEN.pargc = pargc;
    GRAPH->TOKEN.argv = argv;

    // gather session info, including starttime.
    //    subsequent calls to session() just reuse the info gathered in this first call.
    session(SESSION);
    
    // create (or reopen) store for the containers
    SESSION->ikea_store = ikea_store_open( NULL );
#if 1
    GRAPH->ikea_store = ikea_store_open( NULL );
#endif

//    emit_initialize(GRAPH);

    return SESSION;
}

/**
 * finalize and free context
 *
 * @param SESSION context
 */
void finalize( SESSION_t * SESSION )
{
   CONTAINER_t *CONTAINER = SESSION->CONTAINER;
   GRAPH_t *GRAPH = (GRAPH_t*)CONTAINER;
   emit_finalize(GRAPH);

   ikea_store_snapshot(GRAPH->ikea_store);
   ikea_store_close(GRAPH->ikea_store);

   // free context
   free(GRAPH);
   free(SESSION);
}

void interrupt( SESSION_t * SESSION )
{
   CONTAINER_t *CONTAINER = SESSION->CONTAINER;
   GRAPH_t *GRAPH = (GRAPH_t*)CONTAINER;
   ikea_store_close(GRAPH->ikea_store);
}
