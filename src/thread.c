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
#include "merge.h"

THREAD_t * thread(PROCESS_t *PROCESS, int *pargc, char *argv[], int optind)
{
    THREAD_t thread = { 0 }; // includes castable: TOKEN_t, IO_t, LIST_t, INBUF_t
    THREAD_t *THREAD = &thread;    // needed for LIST() and E() macros.

    argv = &argv[optind];
    *pargc -= optind;

    if (*pargc == 0 && ! (PROCESS->flags & 16)) {    // No file args, or commandline acts,
                          //    default to stdin
        argv[0] = "-";
        *pargc = 1;
    }

    thread.PROCESS = PROCESS;
    thread.TOKEN.IO.LIST.INBUF.PROC_INBUF = &(PROCESS->PROC_INBUF);
    thread.TOKEN.IO.LIST.PROC_LIST = &(PROCESS->PROC_LIST);
    thread.TOKEN.IO.out = stdout;
    thread.TOKEN.IO.err = stderr;
    thread.TOKEN.IO.pargc = pargc;
    thread.TOKEN.IO.argv = argv;
    thread.TOKEN.IO.acts = PROCESS->acts;

// FIXME - fork() here ??
    // run until completion

    (void) container(&thread);

#if 0
    merge(THREAD, "12345", "abcde");
    merge(THREAD, "98765", "zyxwv");
    merge(THREAD, "12345", "abcde");
    merge(THREAD, "98765", "zyxwv");
#endif
    // Print the top container

    // do this for canonical g output
    // alternatively (based on command line options),
    // process through parser
    // and pretty printer, or gv converter

    // FIXME - move this to a function in io.c, or ikea.c
 
    char buf[1024];
    FILE *fh = ikea_box_fopen(
            PROCESS->ikea_store,
            thread.TOKEN.IO.contenthash, "r");
    if (fh) {
        size_t len;
        while ( (len = fread(buf, 1, sizeof(buf), fh)) ) {
            (void) fwrite(buf, 1, len, stdout);
        }
        fclose(fh);
    }

// FIXME - do this only if we are the last thread exiting ...
    free_tree(LIST(), PROCESS->merge_cache);
    free_tree(LIST(), PROCESS->identifiers);

    // check that everything has been freed
    if (LIST()->PROC_LIST->stat_elemnow != 0) {
        E();
        assert(0);
    }

    return NULL;   // FIXME - presumably some kind of thread handle...
}
