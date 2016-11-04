/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

// private includes
#include "ikea.h"
#include "thread.h"

// public include
#include "libje.h"

success_t parse(THREAD_t * THREAD)
{
    GRAPH_t *GRAPH = (GRAPH_t*)(THREAD->CONTAINER);
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
THREAD_t *initialize(int *pargc, char *argv[], int optind)
{
    THREAD_t *THREAD;
    CONTAINER_t *CONTAINER;
    GRAPH_t *GRAPH;
    TOKEN_t *TOKEN;

    // FIXME - I think THREAD and CONTAINER can just be onthe stack of THREAD ??
    //
    if (! (THREAD = calloc(1, sizeof(THREAD_t))))
        FATAL("calloc()");

    THREAD->progname = argv[0];
    THREAD->out = stdout;
    TOKEN = (TOKEN_t*)THREAD;

    if (! (CONTAINER = calloc(1, sizeof(CONTAINER_t))))
        FATAL("calloc()");

    GRAPH = (GRAPH_t*)CONTAINER;
    GRAPH->THREAD = THREAD;
    THREAD->CONTAINER = CONTAINER;
#if 1
    THREAD->progname = argv[0];
#endif

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0) {    // No file args,  default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    TOKEN->pargc = pargc;
    TOKEN->argv = argv;

    // gather session info, including starttime.
    //    subsequent calls to session() just reuse the info gathered in this first call.
    session(THREAD);
    
    // create (or reopen) store for the containers
    THREAD->ikea_store = ikea_store_open( NULL );
#if 1
    GRAPH->ikea_store = ikea_store_open( NULL );
#endif
    return THREAD;
}

/**
 * finalize and free context
 *
 * @param THREAD context
 */
void finalize( THREAD_t * THREAD )
{
   CONTAINER_t *CONTAINER = THREAD->CONTAINER;
   GRAPH_t *GRAPH = (GRAPH_t*)CONTAINER;

   ikea_store_snapshot(GRAPH->ikea_store);
   ikea_store_close(GRAPH->ikea_store);

   // free context
   free(GRAPH);
   free(THREAD);
}

void interrupt( THREAD_t * THREAD )
{
   CONTAINER_t *CONTAINER = THREAD->CONTAINER;
   GRAPH_t *GRAPH = (GRAPH_t*)CONTAINER;
   ikea_store_close(GRAPH->ikea_store);
}
