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
#include "input.h"

#define INBUF() ((INBUF_t*)INPUT)
#define LIST() ((LIST_t*)INPUT)

/**
 * fill buffers from input files
 *
 * @param INPUT context
 * @return success/fail
 */
success_t input(INPUT_t * INPUT)
{
    int size, avail;

    if (INBUF()->inbuf) {        // if there is an existing active-inbuf
        if (INPUT->in == (INBUF()->inbuf->buf + INBUFSIZE)) {    // if it is full
            if ((--(INBUF()->inbuf->refs)) == 0) {    // dereference active-inbuf
                free_inbuf(INBUF(), INBUF()->inbuf);    // free if no refs left
                         // can happen it held only framents that were ignore, or
                         // which were copied out into SHORTSTRELEM
            }
            INBUF()->inbuf = new_inbuf(INBUF());    // get new
            assert(INBUF()->inbuf);
            INBUF()->inbuf->refs = 1;    // add active-inbuf reference
            INPUT->in = INBUF()->inbuf->buf;    // point to beginning of buffer
        }
    } else {        // no inbuf, implies just starting
        INBUF()->inbuf = new_inbuf(INBUF());    // get new
        assert(INBUF()->inbuf);
        INBUF()->inbuf->refs = 1;    // add active-inbuf reference
        INPUT->in = INBUF()->inbuf->buf;
    }
    if (INPUT->file) {        // if there is an existing active input file
        if (INPUT->in == INPUT->end && feof(INPUT->file)) {    //    if it is at EOF
            if (INPUT->file != stdin) {
                fclose(INPUT->file);    // then close it and indicate no active input file
            }
            INPUT->file = NULL;
        }
    }
    if (!INPUT->file) {        // if there is no active input file
        if (*(INPUT->pargc) > 0) {    //   then try to open the next file
            INPUT->filename = INPUT->argv[0];
            (*(INPUT->pargc))--;
            INPUT->argv = &(INPUT->argv[1]);
            if (strcmp(INPUT->filename, "-") == 0) {
                INPUT->file = stdin;
                *(INPUT->pargc) = 0;    // No files after stdin
#if 0
// FIXME - delete
            } else if (strcmp(INPUT->filename, "-e") == 0) {
                INPUT->membuf = INPUT->argv[0];
                (*(INPUT->pargc))--;
                if (INPUT->membuf) fprintf(stderr, "NOT YET WORKING:  %s\n", INPUT->membuf);
                    return FAIL;    // no more input available
#endif
            } else {
                if (! (INPUT->file = fopen(INPUT->filename, "rb"))) {
                    FATAL("fopen(\"%s\", \"rb\")", INPUT->filename);
                }
            }
            INPUT->linecount_at_start = INPUT->stat_lfcount ? INPUT->stat_lfcount : INPUT->stat_crcount;
            INPUT->stat_infilecount++;
        } else if (INPUT->acts) {
// FIXME - how to get string into input ???
            fprintf(stderr,"process command line\n");
        } else {
            return FAIL;    // no more input available
        }
// FIXME - assert(INPUT->file || INPUT->acts);
        assert(INPUT->file);
    }
// FIXME  -- or slurp in data from acts
    // slurp in data from file stream
    avail = INBUF()->inbuf->buf + INBUFSIZE - INPUT->in;
    size = fread(INPUT->in, 1, avail, INPUT->file);
    INPUT->end = INPUT->in + size;

    if (size == 0 && ! feof(INPUT->file)) {
        if (ferror(INPUT->file)) {
            FATAL("fread()");
        }
    }

    INPUT->stat_incharcount += size;
    return SUCCESS;
}
