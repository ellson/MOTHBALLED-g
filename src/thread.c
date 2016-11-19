/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"

THREAD_t * thread(SESSION_t *SESSION, int *pargc, char *argv[], int optind, char* acts)
{
    THREAD_t thread = { 0 };      // FIXME - may need to calloced
    THREAD_t *THREAD = &thread;    // needed for LIST() and E() macros.

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0 && !acts) {    // No file args, or commandline acts,  default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    thread.SESSION = SESSION;
    thread.TOKEN.out = stdout;
    thread.TOKEN.err = stderr;
    thread.TOKEN.pargc = pargc;
    thread.TOKEN.argv = argv;
    thread.TOKEN.acts = acts;
    thread.ikea_store = ikea_store_open( NULL );

// FIXME - fork() here ??
    // run until completion
    (void)container(&thread);

    ikea_store_snapshot(thread.ikea_store);
    ikea_store_close(thread.ikea_store);

    free_tree(LIST(), thread.attrid);

    if (LIST()->stat_elemnow != 0) {
        E();
        assert(0);
    }

    return NULL;   // FIXME - presumably some kind of thread handle...
}
