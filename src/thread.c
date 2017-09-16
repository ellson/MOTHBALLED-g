/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

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

    if (*pargc == 0 && ! (PROCESS->flags & 16)) {    // No file args, or commandline acts,
                          //    default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    thread.PROCESS = PROCESS;
    thread.TOKEN.IO.out = stdout;
    thread.TOKEN.IO.err = stderr;
    thread.TOKEN.IO.pargc = pargc;
    thread.TOKEN.IO.argv = argv;
    thread.TOKEN.IO.acts = PROCESS->acts;
    thread.TOKEN.IO.ikea_store = ikea_store_open( NULL ); // FIXME - belongs in process?

// FIXME - fork() here ??
    // run until completion
    elem_t *content = container(&thread);
//P(content);

#if 0
    // write to stdout
    elem_t *nodes = content->u.l.first;
    elem_t *edges = nodes->u.l.next;
    elem_t *tree = nodes->u.l.first;
    if (tree) {
        IO()->flags = THREAD->PROCESS->flags;
        IO()->out_disc = &stdout_disc;
        IO()->out_chan = IO()->out_disc->out_open_fn( NULL, NULL );
        printt(IO(), tree);
        free_tree(LIST(), tree);
        nodes->u.l.first = NULL;

        tree = edges->u.l.first;
        if (tree) {
            printt(IO(), tree);
            free_tree(LIST(), tree);
            edges->u.l.first = NULL;
        }

        IO()->out_disc->out_flush_fn(IO());
        IO()->out_disc->out_close_fn(IO());
    }
#else
    // FIXME - do this properly!
 
    char buf[10000];
    FILE *fh = ikea_box_fopen(thread.TOKEN.IO.ikea_store, thread.TOKEN.IO.contenthash, "r");
    if (fh) {
        size_t len = fread(buf, 1, sizeof(buf), fh);
        (void) fwrite(buf, 1, len, stdout);
        fclose(fh);
    }

#endif

    free_list(LIST(), content);

    ikea_store_snapshot(thread.TOKEN.IO.ikea_store);   // FIXME - belongs in process?
    ikea_store_close(thread.TOKEN.IO.ikea_store);      // FIXME - belongs in process?

// FIXME - do this only if we are the last thread exiting ...
    free_tree(LIST(), PROCESS->identifiers);

    if (LIST()->stat_elemnow != 0) {
        E();
        assert(0);
    }

    return NULL;   // FIXME - presumably some kind of thread handle...
}
