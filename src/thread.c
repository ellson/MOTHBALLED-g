/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "thread.h"

THREAD_t * thread(SESSION_t *SESSION, int *pargc, char *argv[], int optind)
{
    THREAD_t thread = { 0 };      // FIXME - may need to calloced
    success_t rc;

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0) {    // No file args,  default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    thread.SESSION = SESSION;
    thread.out = stdout;
    thread.TOKEN.pargc = pargc;
    thread.TOKEN.argv = argv;
    thread.ikea_store = ikea_store_open( NULL );

// FIXME - fork() here ??
    // run until completion
    rc = container(&thread);

    ikea_store_snapshot(thread.ikea_store);
    ikea_store_close(thread.ikea_store);

    return NULL;   // FIXME - presumably some kind of thread handle...
}
