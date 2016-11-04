/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "thread.h"

void thread(SESSION_t *SESSION, int *pargc, char *argv[])
{
    THREAD_t thread_s = { 0 };
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

    // gather session info, including starttime.
    //    subsequent calls to session() just reuse the info gathered in this first call.
    session(THREAD);
    
    // create (or reopen) store for the containers
    THREAD->ikea_store = ikea_store_open( NULL );

    // run until completion
    rc = container(THREAD);

    ikea_store_snapshot(THREAD->ikea_store);
    ikea_store_close(THREAD->ikea_store);

    // free context
#if 0
// FIXME
    free(GRAPH);
    free(THREAD);
#endif
}
