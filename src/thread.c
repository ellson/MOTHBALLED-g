/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "thread.h"

THREAD_t * thread(SESSION_t *SESSION, int *pargc, char *argv[], int optind)
{
    THREAD_t thread_s = { 0 };      // FIXME - may need to calloced
                                    // if thread() just starts the thread
    THREAD_t *THREAD = &thread_s;
    TOKEN_t *TOKEN = (TOKEN_t*)THREAD;

    THREAD->out = stdout;
    THREAD->err = stderr;

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0) {    // No file args,  default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    TOKEN->pargc = pargc;
    TOKEN->argv = argv;

    // create (or reopen) store for the containers
    THREAD->ikea_store = ikea_store_open( NULL );

// FIXME - fork() here ??
    // run until completion
    rc = container(THREAD);

    ikea_store_snapshot(THREAD->ikea_store);
    ikea_store_close(THREAD->ikea_store);

    // clean up
#if 0
// FIXME
    free(GRAPH);
    free(THREAD);
#endif

    return NULL;
}
