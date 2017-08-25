/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"

THREAD_t * thread(PROCESS_t *PROCESS, int *pargc, char *argv[], int optind)
{
    THREAD_t thread = { 0 };      // FIXME - may need to calloced
    THREAD_t *THREAD = &thread;    // needed for LIST() and E() macros.

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0) {    // No file args, or commandline acts,
                          //    default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    thread.PROCESS = PROCESS;
    thread.TOKEN.out = stdout;
    thread.TOKEN.err = stderr;
    thread.TOKEN.pargc = pargc;
    thread.TOKEN.argv = argv;
    thread.ikea_store = ikea_store_open( NULL );

// FIXME - fork() here ??
    // run until completion
    (void)container(&thread);

    ikea_store_snapshot(thread.ikea_store);
    ikea_store_close(thread.ikea_store);

    free_tree(LIST(), thread.identifiers);

    if (LIST()->stat_elemnow != 0) {
        E();
        assert(0);
    }

    return NULL;   // FIXME - presumably some kind of thread handle...
}
