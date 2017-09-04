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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "types.h"
#include "fatal.h"
#include "inbuf.h"
#include "list.h"
#include "io.h"

#define INBUF() ((INBUF_t*)IO)
#define LIST() ((LIST_t*)IO)

/**
 * fill buffers from input files
 *
 * @param IO context
 * @return success/fail
 */
success_t input(IO_t * IO)
{
    int size, avail;

    if (INBUF()->inbuf) {        // if there is an existing active-inbuf
        if (IO->in == (INBUF()->inbuf->buf + INBUFSIZE)) {    // if it is full
            if ((--(INBUF()->inbuf->refs)) == 0) {    // dereference active-inbuf
                free_inbuf(INBUF(), INBUF()->inbuf);    // free if no refs left
                         // can happen it held only framents that were ignore, or
                         // which were copied out into SHORTSTRELEM
            }
            INBUF()->inbuf = new_inbuf(INBUF());    // get new
            assert(INBUF()->inbuf);
            INBUF()->inbuf->refs = 1;    // add active-inbuf reference
            IO->in = INBUF()->inbuf->buf;    // point to beginning of buffer
        }
    } else {        // no inbuf, implies just starting
        INBUF()->inbuf = new_inbuf(INBUF());    // get new
        assert(INBUF()->inbuf);
        INBUF()->inbuf->refs = 1;    // add active-inbuf reference
        IO->in = INBUF()->inbuf->buf;
    }
    if (IO->file) {        // if there is an existing active input file
        if (IO->in == IO->end && feof(IO->file)) {    //    if it is at EOF
            if (IO->file != stdin) {
                fclose(IO->file);    // then close it and indicate no active input file
            }
            IO->file = NULL;
        }
    }
    if (!IO->file) {        // if there is no active input file
        if (*(IO->pargc) > 0) {    //   then try to open the next file
            IO->filename = IO->argv[0];
            (*(IO->pargc))--;
            IO->argv = &(IO->argv[1]);
            if (strcmp(IO->filename, "-") == 0) {
                IO->file = stdin;
                *(IO->pargc) = 0;    // No files after stdin
#if 0
// FIXME - delete
            } else if (strcmp(IO->filename, "-e") == 0) {
                IO->membuf = IO->argv[0];
                (*(IO->pargc))--;
                if (IO->membuf) fprintf(stderr, "NOT YET WORKING:  %s\n", IO->membuf);
                    return FAIL;    // no more input available
#endif
            } else {
                if (! (IO->file = fopen(IO->filename, "rb"))) {
                    FATAL("fopen(\"%s\", \"rb\")", IO->filename);
                }
            }
            IO->linecount_at_start = IO->stat_lfcount ? IO->stat_lfcount : IO->stat_crcount;
            IO->stat_infilecount++;
        } else if (IO->acts) {
// FIXME - how to get string into input ???
            fprintf(stderr,"process command line\n");
        } else {
            return FAIL;    // no more input available
        }
// FIXME - assert(IO->file || IO->acts);
        assert(IO->file);
    }
// FIXME  -- or slurp in data from acts
    // slurp in data from file stream
    avail = INBUF()->inbuf->buf + INBUFSIZE - IO->in;
    size = fread(IO->in, 1, avail, IO->file);
    IO->end = IO->in + size;

    if (size == 0 && ! feof(IO->file)) {
        if (ferror(IO->file)) {
            FATAL("fread()");
        }
    }

    IO->stat_incharcount += size;
    return SUCCESS;
}
